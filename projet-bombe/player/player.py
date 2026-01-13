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


class Player:
    def __init__(self):
        self.sock = None
        self.alive = True
        self.has_bomb = False
        self.other_players = []
        self.running = True

    def connect(self):
        for i in range(10):
            try:
                self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.sock.connect((SERVER_HOST, SERVER_PORT))
                print(f"Connecte au serveur {SERVER_HOST}:{SERVER_PORT}")
                return True
            except:
                print(f"Tentative {i+1}/10...")
                time.sleep(2)
        return False

    def register(self):
        self.sock.send(json.dumps({'action': 'register', 'player_id': PLAYER_ID}).encode())
        resp = json.loads(self.sock.recv(4096).decode())
        if resp.get('status') == 'ok':
            print(f"Enregistre comme {PLAYER_ID}")
            return True
        return False

    def pass_bomb(self):
        time.sleep(random.uniform(0.5, 2.0))
        if self.has_bomb and self.alive and self.other_players:
            target = random.choice(self.other_players)
            self.sock.send(json.dumps({'action': 'pass', 'to': target}).encode())
            print(f"Bombe passee a {target}")

    def handle_event(self, msg):
        event = msg.get('event')
        
        if event == 'start':
            self.other_players = [p for p in msg['players'] if p != PLAYER_ID]
            if msg['holder'] == PLAYER_ID:
                self.has_bomb = True
                print(">>> J'AI LA BOMBE! <<<")
                threading.Thread(target=self.pass_bomb, daemon=True).start()
        
        elif event == 'pass':
            if msg['to'] == PLAYER_ID:
                self.has_bomb = True
                print(">>> J'AI LA BOMBE! <<<")
                threading.Thread(target=self.pass_bomb, daemon=True).start()
            elif msg['from'] == PLAYER_ID:
                self.has_bomb = False
        
        elif event == 'new_round':
            if msg['holder'] == PLAYER_ID:
                self.has_bomb = True
                print(">>> J'AI LA BOMBE! <<<")
                threading.Thread(target=self.pass_bomb, daemon=True).start()
        
        elif event == 'explosion':
            if msg['eliminated'] == PLAYER_ID:
                print("BOOM! Je suis elimine...")
                self.alive = False
                self.running = False
            else:
                if msg['eliminated'] in self.other_players:
                    self.other_players.remove(msg['eliminated'])
                print(f"{msg['eliminated']} elimine!")
        
        elif event == 'win':
            if msg['winner'] == PLAYER_ID:
                print("*** J'AI GAGNE! ***")
            else:
                print(f"Gagnant: {msg['winner']}")
            self.running = False

    def run(self):
        if not self.connect() or not self.register():
            return
        
        print("En attente du debut de la partie...")
        while self.running:
            try:
                data = self.sock.recv(4096)
                if not data:
                    break
                self.handle_event(json.loads(data.decode()))
            except:
                break
        
        self.sock.close()


if __name__ == '__main__':
    Player().run()
