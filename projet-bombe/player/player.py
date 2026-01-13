"""Client joueur du jeu de la bombe."""

import socket
import json
import random
import time
import threading
import os

SERVER_HOST = os.environ.get('SERVER_HOST', 'localhost')
SERVER_PORT = int(os.environ.get('SERVER_PORT', 6000))
PLAYER_ID = os.environ.get('PLAYER_ID', f'player_{random.randint(1000,9999)}')

SEP = b'\n'


class Player:
    def __init__(self):
        self.sock = None
        self.alive = True
        self.has_bomb = False
        self.other_players = []
        self.running = True
        self.lock = threading.Lock()

    def connect(self):
        for i in range(10):
            try:
                self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.sock.connect((SERVER_HOST, SERVER_PORT))
                print(f"Connecte a {SERVER_HOST}:{SERVER_PORT}", flush=True)
                return True
            except:
                print(f"Tentative {i+1}/10...", flush=True)
                time.sleep(2)
        return False

    def send(self, msg):
        try:
            self.sock.send(json.dumps(msg).encode() + SEP)
        except:
            pass

    def register(self):
        self.send({'action': 'register', 'player_id': PLAYER_ID})
        buffer = b''
        while SEP not in buffer:
            buffer += self.sock.recv(4096)
        line = buffer.split(SEP, 1)[0]
        resp = json.loads(line.decode())
        if resp.get('status') == 'ok':
            print(f"Enregistre: {PLAYER_ID}", flush=True)
            return True
        return False

    def pass_bomb(self):
        time.sleep(random.uniform(0.5, 2.0))
        with self.lock:
            if self.has_bomb and self.alive and self.other_players:
                target = random.choice(self.other_players)
                self.send({'action': 'pass', 'to': target})
                print(f"Bombe passee a {target}", flush=True)

    def handle_event(self, msg):
        event = msg.get('event')
        
        if event == 'start':
            with self.lock:
                self.alive = True
                self.other_players = [p for p in msg['players'] if p != PLAYER_ID]
                if msg['holder'] == PLAYER_ID:
                    self.has_bomb = True
                    print(">>> J'AI LA BOMBE! <<<", flush=True)
                    threading.Thread(target=self.pass_bomb, daemon=True).start()
                else:
                    self.has_bomb = False
            print(f"Partie lancee! Joueurs: {msg['players']}", flush=True)
        
        elif event == 'pass':
            with self.lock:
                if msg['to'] == PLAYER_ID:
                    self.has_bomb = True
                    print(">>> J'AI LA BOMBE! <<<", flush=True)
                    threading.Thread(target=self.pass_bomb, daemon=True).start()
                elif msg['from'] == PLAYER_ID:
                    self.has_bomb = False
        
        elif event == 'new_round':
            with self.lock:
                self.other_players = [p for p in msg.get('alive', []) if p != PLAYER_ID]
                if msg['holder'] == PLAYER_ID:
                    self.has_bomb = True
                    print(">>> J'AI LA BOMBE! <<<", flush=True)
                    threading.Thread(target=self.pass_bomb, daemon=True).start()
                else:
                    self.has_bomb = False
            print(f"Nouveau round. En vie: {msg.get('alive', [])}", flush=True)
        
        elif event == 'explosion':
            with self.lock:
                eliminated = msg['eliminated']
                if eliminated == PLAYER_ID:
                    print("BOOM! Je suis elimine...", flush=True)
                    self.alive = False
                    self.has_bomb = False
                else:
                    if eliminated in self.other_players:
                        self.other_players.remove(eliminated)
                    print(f"{eliminated} elimine!", flush=True)
        
        elif event == 'win':
            with self.lock:
                self.has_bomb = False
            if msg['winner'] == PLAYER_ID:
                print("*** J'AI GAGNE! ***", flush=True)
            else:
                print(f"Gagnant: {msg['winner']}", flush=True)
        
        elif event == 'reset':
            with self.lock:
                self.alive = True
                self.has_bomb = False
                self.other_players = []
            print("Reset. En attente nouvelle partie...", flush=True)
        
        elif event == 'join':
            print(f"{msg['player']} a rejoint", flush=True)

    def run(self):
        if not self.connect() or not self.register():
            return
        
        print("En attente du debut...", flush=True)
        buffer = b''
        
        while self.running:
            try:
                data = self.sock.recv(4096)
                if not data:
                    print("Deconnecte du serveur", flush=True)
                    break
                
                buffer += data
                
                while SEP in buffer:
                    line, buffer = buffer.split(SEP, 1)
                    if line:
                        self.handle_event(json.loads(line.decode()))
                        
            except Exception as e:
                print(f"Erreur: {e}", flush=True)
                break
        
        self.sock.close()


if __name__ == '__main__':
    Player().run()