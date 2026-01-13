"""Client joueur standalone pour Windows."""

import socket
import json
import random
import time
import threading
import sys

def main():
    if len(sys.argv) < 2:
        print("Usage: python player_windows.py <SERVER_IP> [PLAYER_ID]")
        sys.exit(1)
    
    server_host = sys.argv[1]
    player_id = sys.argv[2] if len(sys.argv) > 2 else f"Windows-{random.randint(100,999)}"
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    print(f"Connexion a {server_host}:6000...")
    for i in range(10):
        try:
            sock.connect((server_host, 6000))
            break
        except:
            print(f"Tentative {i+1}/10...")
            time.sleep(2)
    else:
        print("Impossible de se connecter")
        return
    
    sock.send(json.dumps({'action': 'register', 'player_id': player_id}).encode())
    resp = json.loads(sock.recv(4096).decode())
    if resp.get('status') != 'ok':
        print("Erreur d'enregistrement")
        return
    
    print(f"Enregistre comme {player_id}")
    print("En attente du debut...")
    
    has_bomb = False
    alive = True
    other_players = []
    
    def pass_bomb():
        nonlocal has_bomb
        time.sleep(random.uniform(0.5, 2.0))
        if has_bomb and alive and other_players:
            target = random.choice(other_players)
            sock.send(json.dumps({'action': 'pass', 'to': target}).encode())
            print(f"Bombe passee a {target}")
    
    while alive:
        try:
            data = sock.recv(4096)
            if not data:
                break
            msg = json.loads(data.decode())
            event = msg.get('event')
            
            if event == 'start':
                other_players = [p for p in msg['players'] if p != player_id]
                if msg['holder'] == player_id:
                    has_bomb = True
                    print(">>> J'AI LA BOMBE! <<<")
                    threading.Thread(target=pass_bomb, daemon=True).start()
            
            elif event == 'pass':
                if msg['to'] == player_id:
                    has_bomb = True
                    print(">>> J'AI LA BOMBE! <<<")
                    threading.Thread(target=pass_bomb, daemon=True).start()
                elif msg['from'] == player_id:
                    has_bomb = False
            
            elif event == 'new_round':
                if msg['holder'] == player_id:
                    has_bomb = True
                    print(">>> J'AI LA BOMBE! <<<")
                    threading.Thread(target=pass_bomb, daemon=True).start()
            
            elif event == 'explosion':
                if msg['eliminated'] == player_id:
                    print("BOOM! Elimine...")
                    alive = False
                else:
                    if msg['eliminated'] in other_players:
                        other_players.remove(msg['eliminated'])
                    print(f"{msg['eliminated']} elimine!")
            
            elif event == 'win':
                print(f"Gagnant: {msg['winner']}" + (" (MOI!)" if msg['winner'] == player_id else ""))
                break
        except:
            break
    
    sock.close()
    input("Appuyer sur Entree pour quitter...")

if __name__ == '__main__':
    main()
