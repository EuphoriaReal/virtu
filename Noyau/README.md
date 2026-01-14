# TP Modules Noyau Linux

## Prérequis

```bash
sudo apt install linux-headers-$(uname -r)
```

Configuration redémarrage auto après panic :

```bash
echo "kernel.panic = 10" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

---

## Structure

```
kernel_demo/
├── hello/      # Hello world simple
├── crash/      # Overflow de tableau (Oops)
└── panic/      # Kernel panic volontaire
```

---

## Utilisation

### Module Hello

```bash
cd hello
make
sudo insmod mymodule.ko
sudo dmesg | tail -5          # Voir "Hello, world !!!"
sudo rmmod mymodule
sudo dmesg | tail -5          # Voir "Goodbye, world!"
make clean
```

### Module Crash (Oops)

```bash
cd crash
make
sudo insmod crashmodule.ko
sudo dmesg | tail -50         # Observer le Oops
sudo rmmod crashmodule        # Peut échouer si module corrompu
```

### Module Panic

```bash
cd panic
make
sudo insmod panicmodule.ko    # LA VM VA PLANTER
# Attendre 10s, la VM redémarre
```

## Nettoyage

```bash
cd hello && make clean
cd ../crash && make clean
cd ../panic && make clean
```
