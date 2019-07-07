import libperform
import dis


a=[]
total=0

def init(n):
    global a
    for i in range(n):
        a.append(i)

def dead(n):
    global a
    for i in range(n):
        a[i] = i

def kill(n):
    global a
    for i in range(n):
        a[i] = i

def use(n):
    global total
    global a
    for i in range(n):
        total = total + a[i]

def Foo(n):
    dead(n)
    kill(n)
    use(n)

def main():
    print ('testing deadspy on python')
    init(1000)
    for i in range(10000):
        Foo(1000)

print ('Disassembly of dead')
dis.dis(dead)
print ('Disassembly of kill')
dis.dis(kill)
print ('Disassembly of Use')
dis.dis(use)

if __name__ == "__main__":
    main()
