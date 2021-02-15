/* A)Como em_e e em_l são utilizados para fazer a exclusão mútua, ambos devem ser inicializado com o valor 1. 
       O semáforo leit também deve ser iniciado com o valor 1, pois é o responsável pela prioridade da leitura, e por último, o semáforo escr também deve ser
       iniciado com o valor 1 para garantir a exclusão mútua na hora da leitura.
  */
  
  // B)Não, pois para manter a prioridade dos escritores é necessário manter todos os mecanismos de sincronização.
  
  // E)Não recursivos, pois logo após um blqueio é sempre chamado o desbloqueio antes de a thread acabe se bloqueando novamente.
