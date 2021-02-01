class Buffer{

    private int tamanhoBuffer;
    private int count = 0; // quantidade de elementos no buffer
    private int in = 0; // posicão do buffer onde as threads produtoras vão fazer a inserção
    private int out = 0; // posição do buffer onde as threads consumidoras vão fazer a remoção

    public Buffer(int tamanhoBuffer){
        this.tamanhoBuffer = tamanhoBuffer;
    }

    int[] buffer = new int[tamanhoBuffer];

    public void iniciaBuffer(){
        for(int i=0; i<this.tamanhoBuffer; i++){
            this.alteraElemento(i, i);
        }
    }

    public synchronized void entraProdutor(int id){
        try{
            while(this.count == tamanhoBuffer){ // não pode escrever se o vetor estiver cheio
                System.out.println("pc.Produtor("+id+")bloqueado");
                wait();
            }
            this.count++; // buffer contém mais um elemento
            System.out.println("pc.Thread("+id+")produzindo");
        } catch(InterruptedException e) {}
    }

    public synchronized void saiProdutor(int id){            
        in = (in + 1) % tamanhoBuffer; // in recebe a próxima posição do vetor a ser escrita
        notifyAll(); 
        System.out.println("pc.Produtor("+id+")finalizado");
        imprimeBuffer();
    }

    public synchronized void entraConsumidor(int id){
        try{
            while(this.count == 0){ // não pode consumir se o vetor estiver vazio
                System.out.println("pc.Consumidor("+id+")bloqueado");
                wait();
            }
            this.count--; // quantidade de elementos no buffer diminuiu
            System.out.println("pc.Thread("+id+")consumindo");
        } catch(InterruptedException e) {}
    }

    public synchronized void saiConsumidor(int id){
        out = (out + 1) % tamanhoBuffer; // próxima posição do buffer a ser consumida
        notifyAll();
        System.out.println("pc.Consumidor("+id+")finalizado");
        imprimeBuffer();
    }

    public synchronized void imprimeBuffer(){
        System.out.println("----buffer----");
        for(int i=0; i<this.tamanhoBuffer; i++){
            System.out.print(buffer[i]+" ");
        }
        System.out.println();
    }

    public void alteraElemento(int indice, int valor){
        buffer[indice] = valor;
    }

    // metodos para produtores e consumidores terem acesso a in e out
    public int getIn(){
        return this.in;
    }

    public int getOut(){
        return this.out;
    }
}

class Produtor extends Thread{
    int id;
    Buffer buffer;

    Produtor(int id, Buffer buffer){
        this.id = id;
        this.buffer = buffer;
    }

    public void run(){
        this.buffer.entraProdutor(this.id);
        buffer.alteraElemento(buffer.getIn(), this.id); // buffer[in] = id
        this.buffer.saiProdutor(id);
    }
}

class Consumidor extends Thread{
    int id;
    Buffer buffer;

    Consumidor(int id, Buffer buffer){
        this.id = id;
        this.buffer = buffer;
    }

    public void run(){
        buffer.entraConsumidor(this.id);
        buffer.alteraElemento(buffer.getOut(), 0); // buffer[out] = 0
        buffer.saiConsumidor(this.id);
    }
}

class Application{
    static final int produtores = 2;
    static final int consumidores = 2;

    public static void main(String args[]){
        int i;
        Buffer buffer = new Buffer(10); // construtor recebe como parâmetro o tamanho do buffer
        Produtor[] p = new Produtor[produtores];
        Consumidor[] c = new Consumidor[consumidores];

        buffer.iniciaBuffer();

        // log de saida
        System.out.println("import verificaPC");
        System.out.println("PC = verificaPC.PC()");

        for(i=0; i<produtores; i++){
            p[i] = new Produtor(i, buffer);
            p[i].start();
        }

        for(i=0; i<consumidores; i++){
            c[i] = new Consumidor(i, buffer);
            c[i].start();
        }

    }
}
