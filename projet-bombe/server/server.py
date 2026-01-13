"""Serveur du jeu de la bombe."""

import socket
import threading
import json
import random
import os

HOST = os.environ.get('HOST', '0.0.0.0')
PORT = int(os.environ.get('PORT', 6000))
MIN_PLAYERS = int(os.environ.get('MIN_PLAYERS', 3))
BOMB_TIME_MIN = float(os.environ.get('BOMB_TIME_MIN', 5.0))
BOMB_TIME_MAX = float(os.environ.get('BOMB_TIME_MAX', 15.0))


class Server:
    def __init__(self):
        self.players = {}  # {player_id: socket}
        self.alive = []
        self.bomb_holder = None
        self.running = True
        self.game_started = False
        self.lock = threading.Lock()

    def broadcast(self, msg, exclude=None):
        data = json.dumps(msg).encode()
        for pid, sock in self.players.items():
            if pid != exclude:
                try:
                    sock.send(data)
                except:
                    pass

    def handle_client(self, sock, addr):
        player_id = None
        try:
            while self.running:
                data = sock.recv(4096)
                if not data:
                    break
                msg = json.loads(data.decode())
                
                if msg['action'] == 'register':
                    player_id = msg['player_id']
                    with self.lock:
                        self.players[player_id] = sock
                        self.alive.append(player_id)
                    print(f"[+] {player_id} connecte ({len(self.players)} joueurs)")
                    sock.send(json.dumps({'status': 'ok'}).encode())
                    self.broadcast({'event': 'join', 'player': player_id}, player_id)
                
                elif msg['action'] == 'pass':
                    target = msg['to']
                    with self.lock:
                        if player_id == self.bomb_holder and target in self.alive:
                            self.bomb_holder = target
                            print(f"[>] Bombe: {player_id} -> {target}")
                            self.broadcast({'event': 'pass', 'from': player_id, 'to': target})
        except:
            pass
        finally:
            with self.lock:
                if player_id and player_id in self.players:
                    del self.players[player_id]
                    if player_id in self.alive:
                        self.alive.remove(player_id)
                    print(f"[-] {player_id} deconnecte")

    def explode(self):
        with self.lock:
            if not self.game_started or len(self.alive) <= 1:
                return
            
            eliminated = self.bomb_holder
            self.alive.remove(eliminated)
            print(f"[X] BOOM! {eliminated} elimine!")
            self.broadcast({'event': 'explosion', 'eliminated': eliminated})
            
            if len(self.alive) == 1:
                winner = self.alive[0]
                print(f"[*] Gagnant: {winner}")
                self.broadcast({'event': 'win', 'winner': winner})
                self.game_started = False
            else:
                self.bomb_holder = random.choice(self.alive)
                print(f"[o] Nouveau porteur: {self.bomb_holder}")
                self.broadcast({'event': 'new_round', 'holder': self.bomb_holder})
                self.start_timer()

    def start_timer(self):
        delay = random.uniform(BOMB_TIME_MIN, BOMB_TIME_MAX)
        print(f"[~] Explosion dans {delay:.1f}s")
        timer = threading.Timer(delay, self.explode)
        timer.daemon = True
        timer.start()

    def start_game(self):
        with self.lock:
            if len(self.players) < MIN_PLAYERS:
                print(f"[!] Pas assez de joueurs ({len(self.players)}/{MIN_PLAYERS})")
                return
            if self.game_started:
                print("[!] Partie deja en cours")
                return
            
            self.game_started = True
            self.alive = list(self.players.keys())
            self.bomb_holder = random.choice(self.alive)
            
            print(f"[*] Partie lancee avec {len(self.alive)} joueurs")
            print(f"[o] Porteur initial: {self.bomb_holder}")
            self.broadcast({'event': 'start', 'holder': self.bomb_holder, 'players': self.alive})
            self.start_timer()

    def run(self):
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind((HOST, PORT))
        server.listen(10)
        
        print(f"Serveur demarre sur {HOST}:{PORT}")
        print(f"En attente de {MIN_PLAYERS} joueurs minimum")
        print("Commandes: start, status, quit\n")
        
        threading.Thread(target=self.accept_loop, args=(server,), daemon=True).start()
        
        while self.running:
            try:
                cmd = input().strip().lower()
                if cmd == 'start':
                    self.start_game()
                elif cmd == 'status':
                    print(f"Joueurs: {list(self.players.keys())}")
                    print(f"En vie: {self.alive}")
                    print(f"Porteur: {self.bomb_holder}")
                elif cmd == 'quit':
                    self.running = False
            except (EOFError, KeyboardInterrupt):
                break
        
        server.close()

    def accept_loop(self, server):
        while self.running:
            try:
                sock, addr = server.accept()
                threading.Thread(target=self.handle_client, args=(sock, addr), daemon=True).start()
            except:
                break


if __name__ == '__main__':
    Server().run()
