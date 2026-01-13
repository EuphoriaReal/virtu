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
    
    lock = threading.Lock()
    state = {
        'has_bomb': False,
        'alive': True,
        'other_players': [],
        'running': True
    }
    
    def pass_bomb():
        time.sleep(random.uniform(0.5, 2.0))
        with lock:
            if state['has_bomb'] and state['alive'] and state['other_players']:
                target = random.choice(state['other_players'])
                try:
                    sock.send(json.dumps({'action': 'pass', 'to': target}).encode())
                    print(f"Bombe passee a {target}")
                except:
                    pass
    
    while state['running']:
        try:
            data = sock.recv(4096)
            if not data:
                print("Deconnecte du serveur")
                break
            msg = json.loads(data.decode())
            event = msg.get('event')
            
            if event == 'start':
                with lock:
                    state['alive'] = True
                    state['other_players'] = [p for p in msg['players'] if p != player_id]
                    if msg['holder'] == player_id:
                        state['has_bomb'] = True
                        print(">>> J'AI LA BOMBE! <<<")
                        threading.Thread(target=pass_bomb, daemon=True).start()
                    else:
                        state['has_bomb'] = False
                print(f"Partie lancee! Joueurs: {msg['players']}")
            
            elif event == 'pass':
                with lock:
                    if msg['to'] == player_id:
                        state['has_bomb'] = True
                        print(">>> J'AI LA BOMBE! <<<")
                        threading.Thread(target=pass_bomb, daemon=True).start()
                    elif msg['from'] == player_id:
                        state['has_bomb'] = False
            
            elif event == 'new_round':
                with lock:
                    state['other_players'] = [p for p in msg.get('alive', []) if p != player_id]
                    if msg['holder'] == player_id:
                        state['has_bomb'] = True
                        print(">>> J'AI LA BOMBE! <<<")
                        threading.Thread(target=pass_bomb, daemon=True).start()
                    else:
                        state['has_bomb'] = False
                print(f"Nouveau round. En vie: {msg.get('alive', [])}")
            
            elif event == 'explosion':
                with lock:
                    eliminated = msg['eliminated']
                    if eliminated == player_id:
                        print("BOOM! Je suis elimine...")
                        state['alive'] = False
                        state['has_bomb'] = False
                    else:
                        if eliminated in state['other_players']:
                            state['other_players'].remove(eliminated)
                        print(f"{eliminated} elimine!")
            
            elif event == 'win':
                with lock:
                    state['has_bomb'] = False
                if msg['winner'] == player_id:
                    print("*** J'AI GAGNE! ***")
                else:
                    print(f"Gagnant: {msg['winner']}")
            
            elif event == 'reset':
                with lock:
                    state['alive'] = True
                    state['has_bomb'] = False
                    state['other_players'] = []
                print("Reset. En attente nouvelle partie...")
            
            elif event == 'join':
                print(f"{msg['player']} a rejoint")
                
        except Exception as e:
            print(f"Erreur: {e}")
            break
    
    sock.close()
    input("Appuyer sur Entree pour quitter...")


if __name__ == '__main__':
    main()