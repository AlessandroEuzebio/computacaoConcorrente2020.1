class Vetor{

    int[] vetor = new int[10];

    public void imprimeVetor(){
        for(int i=0; i<vetor.length; i++){
            System.out.print(vetor[i]+" ");
        }
        System.out.println();
    }

    public int getElemento(int elemento){
        return this.vetor[elemento];
    }

    public void tamanhoVetor(){
        System.out.println(vetor.length);
    }

    public void alteraElemento(int indice, int valor){
        vetor[indice] = valor;
    }
}


class T extends Thread{
    private int id;
    private int inicio, fim; // inicio e fim do processamento do vetor

    Vetor a, b, c;

    public T(int id, Vetor a, Vetor b, Vetor c, int inicio, int fim){
        this.id = id;
        this.a = a;
        this.b = b;
        this.c = c;
        this.inicio = inicio;
        this.fim = fim;
    }

    public void run(){
        System.out.println("Thread " + this.id + " iniciou.");
        for(int i = inicio; i < fim; i++){
            c.alteraElemento(i, a.getElemento(i) + b.getElemento(i));
        }
        System.out.println("Thread " + this.id + " terminou.");
    }

}

class SomaVetores{
    static final int N = 2; // quantidade de threads
    static final int K = 10; // quantidade de elementos do vetor

    public static void main(String args[]){


        Thread[] threads = new Thread[N];

        // cria 3 instancias de Vetor
        Vetor vetorA = new Vetor();
        Vetor vetorB = new Vetor();
        Vetor vetorC = new Vetor();


        // inicializando os vetores a e b
        for(int i=0; i<K; i++){
            vetorA.alteraElemento(i, i);
            vetorB.alteraElemento(i, i);
        }

        // cria as threads
        for (int i=0; i<threads.length; i++) {
            int inicio = K/N * i; // tamanhoDoVetor/quantidadeDeThreads * i
            int fim;

            if(i == K - 1)
                fim = K;
            else
                fim = inicio + K/N;

            threads[i] = new T(i, vetorA, vetorB, vetorC, inicio, fim);
        }

        //inicia as threads
        for (int i=0; i<threads.length; i++) {
            threads[i].start();
        }

        //espera pelo termino de todas as threads
        for (int i=0; i<threads.length; i++) {
            try { threads[i].join(); } catch (InterruptedException e) { return; }
        }

        System.out.println("Vetor C: ");
        vetorC.imprimeVetor();
    }
}
