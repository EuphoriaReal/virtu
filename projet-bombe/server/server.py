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
        self.players = {}
        self.alive = []
        self.bomb_holder = None
        self.running = True
        self.game_started = False
        self.timer = None
        self.lock = threading.Lock()

    def log(self, msg):
        print(msg, flush=True)

    def broadcast(self, msg, exclude=None):
        data = json.dumps(msg).encode()
        for pid, sock in list(self.players.items()):
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
                        if not self.game_started:
                            self.alive.append(player_id)
                    self.log(f"[+] {player_id} connecte ({len(self.players)} joueurs)")
                    sock.send(json.dumps({'status': 'ok'}).encode())
                    self.broadcast({'event': 'join', 'player': player_id}, player_id)
                
                elif msg['action'] == 'pass':
                    target = msg['to']
                    with self.lock:
                        if player_id == self.bomb_holder and target in self.alive and target != player_id:
                            self.bomb_holder = target
                            self.log(f"[>] Bombe: {player_id} -> {target}")
                            self.broadcast({'event': 'pass', 'from': player_id, 'to': target})
        except:
            pass
        finally:
            with self.lock:
                if player_id and player_id in self.players:
                    del self.players[player_id]
                    if player_id in self.alive:
                        self.alive.remove(player_id)
                    self.log(f"[-] {player_id} deconnecte")

    def explode(self):
        start_new_timer = False
        
        with self.lock:
            if not self.game_started or len(self.alive) <= 1:
                return
            
            eliminated = self.bomb_holder
            if eliminated in self.alive:
                self.alive.remove(eliminated)
            
            self.log(f"[X] BOOM! {eliminated} elimine!")
            self.broadcast({'event': 'explosion', 'eliminated': eliminated})
            
            if len(self.alive) == 1:
                winner = self.alive[0]
                self.log(f"[*] Gagnant: {winner}")
                self.broadcast({'event': 'win', 'winner': winner})
                self.game_started = False
                self.log("[*] Tapez 'reset' puis 'start' pour rejouer.")
            elif len(self.alive) > 1:
                self.bomb_holder = random.choice(self.alive)
                self.log(f"[o] Nouveau porteur: {self.bomb_holder}")
                self.broadcast({'event': 'new_round', 'holder': self.bomb_holder, 'alive': list(self.alive)})
                start_new_timer = True
        
        # Hors du lock pour eviter deadlock
        if start_new_timer:
            self.schedule_explosion()

    def schedule_explosion(self):
        delay = random.uniform(BOMB_TIME_MIN, BOMB_TIME_MAX)
        self.log(f"[~] Explosion dans {delay:.1f}s")
        self.timer = threading.Timer(delay, self.explode)
        self.timer.daemon = True
        self.timer.start()

    def start_game(self):
        with self.lock:
            if len(self.players) < MIN_PLAYERS:
                self.log(f"[!] Pas assez de joueurs ({len(self.players)}/{MIN_PLAYERS})")
                return
            if self.game_started:
                self.log("[!] Partie deja en cours")
                return
            
            self.game_started = True
            self.alive = list(self.players.keys())
            self.bomb_holder = random.choice(self.alive)
            
            self.log(f"[*] Partie lancee avec {len(self.alive)} joueurs")
            self.log(f"[o] Porteur initial: {self.bomb_holder}")
            self.broadcast({'event': 'start', 'holder': self.bomb_holder, 'players': list(self.alive)})
        
        # Hors du lock
        self.schedule_explosion()

    def reset_game(self):
        with self.lock:
            if self.timer:
                self.timer.cancel()
                self.timer = None
            self.game_started = False
            self.alive = list(self.players.keys())
            self.bomb_holder = None
            self.log(f"[*] Reset. {len(self.alive)} joueurs prets.")
            self.broadcast({'event': 'reset'})

    def run(self):
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind((HOST, PORT))
        server.listen(10)
        
        self.log(f"Serveur demarre sur {HOST}:{PORT}")
        self.log(f"En attente de {MIN_PLAYERS} joueurs minimum")
        self.log("Commandes: start, status, reset, quit\n")
        
        threading.Thread(target=self.accept_loop, args=(server,), daemon=True).start()
        
        while self.running:
            try:
                cmd = input().strip().lower()
                if cmd == 'start':
                    self.start_game()
                elif cmd == 'status':
                    self.log(f"Connectes: {list(self.players.keys())}")
                    self.log(f"En vie: {self.alive}")
                    self.log(f"Porteur: {self.bomb_holder}")
                elif cmd == 'reset':
                    self.reset_game()
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