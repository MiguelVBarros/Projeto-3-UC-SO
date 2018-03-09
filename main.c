/*--------------------------------------------------------------------
**
**      Projeto SO - exercicio 3
**            Grupo 57
**
**     Carolina Carreira 87641
**        Miguel Barros 87691
**
---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include "matrix2d.h"


// MACROS

#define VALOR_ABS(x) ((x) >= 0 ? (x) : -(x))


// VARIAVEIS GLOBAIS

pthread_mutex_t mutex_barreira[2];
pthread_cond_t esperando[2];
int esperando_barreira[2];

int votos[2];

int iter, trab, n, tam_fatia;

double maxD;

DoubleMatrix2D* matrix[2];


/*--------------------------------------------------------------------
  | Function: tarefa_escravo
  | Description: Função executada por cada tarefa escravo
---------------------------------------------------------------------*/

void *tarefa_escravo(void* args) {
  int id = *((int *) args);
  int atual, prox;
  int i, j;
  double maxValue, dif_desta_iter = 0;

  int linha_inicial = tam_fatia * id + 1;
  int linha_final = tam_fatia * (1 + id);

  for (int it = 0; it < iter; it++) {
    atual = it % 2;
    prox = 1 - it % 2;
    maxValue = 0;

    // Calcular Pontos Internos
    for (i = linha_inicial; i <= linha_final  ; i++) {
      for (j = 1; j <= n ; j++) {
        double val = (dm2dGetEntry(matrix[atual], i, j - 1) +
                      dm2dGetEntry(matrix[atual], i, j + 1) +
                      dm2dGetEntry(matrix[atual], i - 1, j) +
                      dm2dGetEntry(matrix[atual], i + 1, j)) / 4;

        dm2dSetEntry(matrix[prox], i, j, val);

        // Atualizacao da maior diferenca desta iteracao com a anterior
        dif_desta_iter = VALOR_ABS(val - dm2dGetEntry(matrix[atual], i, j));
        maxValue = maxValue > dif_desta_iter ? maxValue : dif_desta_iter;

      }
    }

    // BARREIRA
    // Existem duas barreira e em cada iteracao preparamos a outra e trocamos
    if (pthread_mutex_lock (&(mutex_barreira[atual]))) {
      fprintf(stderr, "\nErro ao fechar mutex\n");
      pthread_exit((void *)(1));
    }

    esperando_barreira[atual] ++;

    // Cada tarefa vota se quer parar de iterar
    if (maxValue < maxD)
      votos[atual]++;

    while (esperando_barreira[atual] < trab ) {
      if (pthread_cond_wait (&esperando[atual], &mutex_barreira[atual])) {
        fprintf(stderr, "\nErro ao esperar por variavel de condicao\n");
        pthread_exit((void *) (1));
      }
    }

    // Ultima tarefa liberta as outras e preparar a proxima barreira
    if (esperando_barreira[atual] == trab) {

      esperando_barreira[atual]++;
      esperando_barreira[prox] = 0;
      votos[prox] = 0;

      if (pthread_cond_broadcast(&esperando[atual])) {
        fprintf(stderr, "\nErro ao fazer broadcast\n");
        pthread_exit((void *)(1));
      }
    }

    // Verificao dos votos das outras tarefas
    if (votos[atual] == trab) {

      // Atualiza o numero de iteracoes corridas para imprimir a matriz certa no fim
      if (id == trab - 1)
        iter = it + 1;

      if (pthread_mutex_unlock(&mutex_barreira[atual])) {
        fprintf(stderr, "\nErro ao desbloquear mutex\n");
        pthread_exit((void *)(1));
      }

      return ((void*) (0));
    }

    if (pthread_mutex_unlock(&mutex_barreira[atual])) {
      fprintf(stderr, "\nErro ao desbloquear mutex\n");
      pthread_exit((void *) (1));
    }
  }

  return ((void *) 0);
}

/*--------------------------------------------------------------------
| Function: parse_integer_or_exit
---------------------------------------------------------------------*/

int parse_integer_or_exit(char const * str, char const * name)
{
  int value;

  if (sscanf(str, "%d", &value) != 1) {
    fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
    exit(1);
  }
  return value;
}

/*--------------------------------------------------------------------
| Function: parse_double_or_exit
---------------------------------------------------------------------*/

double parse_double_or_exit(char const * str, char const * name)
{
  double value;

  if (sscanf(str, "%lf", &value) != 1) {
    fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
    exit(1);
  }
  return value;
}


/*--------------------------------------------------------------------
| Function: main
---------------------------------------------------------------------*/

int main (int argc, char** argv) {
  if (argc < 9) {
    fprintf(stderr, "\nNúmero de Argumentos Inválido.\n");
    fprintf(stderr, "Utilização: heatSim_p2 N tEsq tSup tDir tInf iter trab csz maxD\n\n");
    return 1;
  }

  // Ler Input   
  n = parse_integer_or_exit(argv[1], "n");
  double tEsq = parse_double_or_exit(argv[2], "tEsq");
  double tSup = parse_double_or_exit(argv[3], "tSup");
  double tDir = parse_double_or_exit(argv[4], "tDir");
  double tInf = parse_double_or_exit(argv[5], "tInf");
  iter = parse_integer_or_exit(argv[6], "iter");
  trab = parse_integer_or_exit(argv[7], "trab");
  maxD = parse_double_or_exit(argv[8], "maxD");

  fprintf(stdout, "\nArgumentos:\n"
          " N=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f iter=%d trab=%d maxD=%.3f\n",
          n, tEsq, tSup, tDir, tInf, iter, trab, maxD);


  // Verificacoes de Input 
  if (n < 1 || tEsq < 0 || tSup < 0 || tDir < 0 || tInf < 0 || iter < 1 || maxD < 0) {
    fprintf(stderr, "\nErro: Argumentos invalidos.\n"
            " Lembrar que N >= 1, temperaturas >= 0 e iter >= 1 e maxD >= 0 \n\n");
    return 1;
  }

  if (n % trab != 0) {
    fprintf(stderr, "\nErro: Argumento %s e %s invalidos\n"
            "%s deve ser multiplo de %s.", "N", "trab", "N", "trab");
    return 1;
  }


  int i;
  for (i = 0; i < 2; i++) {
    if (pthread_cond_init (&esperando[i], NULL) ) {
      fprintf(stderr, "\nErro ao inicializar variavel de condicao\n");
      return 1;
    }

    if (pthread_mutex_init (&mutex_barreira[i], NULL)) {
      fprintf(stderr, "\nErro ao inicializar mutex\n");
      return 1;
    }
  }

  // Calcular Tamanho de cada Fatia
  tam_fatia = n / trab;

  matrix[0] = dm2dNew(n + 2, n + 2);
  matrix[1] = dm2dNew(n + 2, n + 2);

  if (matrix[0] == NULL || matrix[1] == NULL) {
    fprintf(stderr, "\nErro: Nao foi possivel alocar memoria para as matrizes.\n\n");
    return 1;
  }

  dm2dSetLineTo (matrix[0], 0, tSup);
  dm2dSetLineTo (matrix[0], n + 1, tInf);
  dm2dSetColumnTo (matrix[0], 0, tEsq);
  dm2dSetColumnTo (matrix[0], n + 1, tDir);

  dm2dCopy (matrix[1], matrix[0]);

  int ids[trab];
  pthread_t * workers = (pthread_t *) malloc(trab * sizeof(pthread_t));

  if (workers == NULL) {
    fprintf(stderr, "\nErro ao alocar memória para threads.\n");
    return 1;
  }

  // Criar Escravos
  for (i = 0; i < trab; i++) {
    ids[i] = i;

    if (pthread_create(&workers[i], NULL, tarefa_escravo, &ids[i])) {
      fprintf(stderr, "\nErro ao criar um escravo.\n");
      return 1;  pthread_exit((void*)(1));
    }
  }


  for (i = 0; i < trab; i++)
    if (pthread_join(workers[i], NULL)) {
      fprintf(stderr, "\nErro ao esperar por uma tarefa\n");
      return 1;
    }


  for (i = 0; i < 2; i++) {
    if (pthread_mutex_destroy(&mutex_barreira[i])) {
      fprintf(stderr, "Erro ao destruir mutex\n");
      return 1;
    }

    if (pthread_cond_destroy (&esperando[i])) {
      fprintf(stderr, "Erro ao destruir variavel de condicao\n");
      return 1;
    }
  }
  

  iter % 2 ? dm2dPrint(matrix[1]) : dm2dPrint(matrix[0]);

  free(workers);

  dm2dFree(matrix[0]);
  dm2dFree(matrix[1]);

  return 0;
}

