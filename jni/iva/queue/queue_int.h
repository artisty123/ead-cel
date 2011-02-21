#ifndef _QUEUE_INT_H_
#define _QUEUE_INT_H_

#include <pthread.h>
#include <list>
using namespace std;

#include "queue.h"


/// \todo Porque essas constantes da netcom t�o aqui?
//#define NETCOM_EVENT                  SDL_USEREVENT + 2
//#define NETCOM_RESET_ENCODE           524


#define QUEUE_MAX_CONSUMERS  100                            ///< N�mero m�ximo de consumidores em uma queue
#define QUEUE_KBYTE          1024                           ///< Quantos bits h� em um KByte
#define QUEUE_BLOCK          (QUEUE_KBYTE*20)               ///< Tamanho (KB) dos blocos de mem�ria
#define QUEUE_CHUNKS         1000                           ///< Quantidade de blocos de mem�ria
#define QUEUE_MEMORY_SYZE    (QUEUE_BLOCK * QUEUE_CHUNKS)   ///< Tamanho m�ximo (KB) da mem�ria para as queues
#define QUEUE_MAX_BLOCKS     40                             ///< N�mero m�ximo de blocos que podem ser alocados

/** \brief Elemento da queue que guarda um buffer de dados e informa��es
 *         associadas a ele
 */
typedef struct queue_store_s {
    struct queue_store_s *nextstore_p;  ///< Pr�ximo elemento
    struct queue_store_s *prevstore_p;  ///< Elemento anterior
    uint8_t *buffer_p;                  ///< Ponteiro para o buffer de dados
    uint32_t buffersize;                ///< Tamanho do buffer de dados
    uint32_t timestamp;                 ///< Timestamp associado ao buffer de dados
    QueueExtraData * data;              ///< Dados adicionais associados � este item da queue
    int consumers;                      ///< N�mero de consumidores que ainda n�o consumiram este elemento
} queue_store_t;

/** \brief Estrutura que representa uma queue
 *
 * <b>Funcionamento b�sico da queue:</b> \n
 * Uma queue � uma fila de itens que podem ser consumidos por um consumidor (queue_consumer_t).
 * A queue � criada com queue_create() e destru�da com queue_destroy(). \n
 * Para pegar dados da queue, � necess�rio registrar um consumidor com queue_registerConsumer(). A
 * partir da�, basta buscar os dados usando queue_dequeue(). Quando um consumidor n�o � mais necess�rio,
 * ele pode ser removido com queue_unregisterConsumer(). \n
 * Para inserir dados na queue n�o � necess�rio registrar um consumidor. Mas se ela n�o possuir
 * nenhum consumidor, o dado inserido ser� ignorado pela queue, ou seja, n�o ser� mantido na queue.
 * Os dados s�o inseridos com queue_enqueue(). \n
 * A queue utiliza uma �rea de mem�ria espec�fica para seus dados para otimizar o desempenho. Esta
 * �rea de mem�ria � alocada em blocos de QUEUE_MEMORY_SYZE (20 MB) bytes sempre que � necess�rio mais
 * mem�ria. Todos os buffers inseridos na queue devem ter sido alocados com a fun��o queue_malloc(), que
 * aloca um bloco desta �rea de mem�ria privada da queue.
 * 
 * <b>Passos para inserir dados na queue:</b> \n
 * \li Alocar dados com queue_malloc()
 * \li Inserir dados com queue_enqueue()
 * \li Se a inser��o dos dados falhar, dados devem ser desalocados com queue_dealloc()
 *
 * <b>Passos para remover dados da queue:</b> \n
 * \li Registrar consumidor com queue_registerConsumer()
 * \li Remover dados com queue_dequeue()
 * \li Assim que o buffer retirado da queue for usado, ele deve ser liberado com queue_free()
 * \li Repetir os 2 passos acima enquanto deseja-se consumir da queue
 * \li Desregistrar o consumidor com queue_unregisterConsumer() quando n�o for mais necess�rio
 * 
 * <b>Como os dados extra funcionam na queue:</b> \n
 * No enqueue, � criada uma C�PIA do objeto QueueExtraData passado por par�metro. Esta c�pia
 * � guardada junto a uma posi��o na queue.
 * No dequeue � retornada uma refer�ncia a este objeto, que N�O deve ser deletado fora da queue.
 * O objeto s� � desalocado (dentro da queue) quando todos os consumidores registrados j� liberaram
 * a posi��o da queue com a qual o objeto est� associado (na fun��o queue_freeDirect()).
 */
typedef struct queue_s {
    queue_store_t *firststore_p;                ///< primeiro elemento da queue
    queue_store_t *laststore_p;                 ///< ultimo elemento da queue
    int length;                                 ///< tamanho da queue (numero de elementos)
    uint32_t size;                              ///< memoria total ocupada pela queue, em bytes
    uint32_t last_timestamp;                    ///< valor do ultimo timestamp adicionado
    uint32_t last_buffersize;                   ///< valor do ultimo buffersize adicionado
    struct queue_consumer_s *firstconsumer_p;   ///< fila de consumidores
    int consumers;                              ///< numero corrente de consumidores
    pthread_mutex_t mtx;                        ///< protege a fila em operacoes criticas  
    pthread_cond_t condVariable;
} queue_t;

/** \brief Consumidor de dados da queue
 */
typedef struct queue_consumer_s {
    struct queue_consumer_s *nextconsumer_p;    ///< Pr�ximo consumidor
    struct queue_s *queue_p;                    ///< Queue da qual o consumidor consome
    struct queue_store_s *store_p;
    int hold;
    uint32_t current_timestamp;                  ///< valor do timestamp atual
} queue_consumer_t;

/** \brief Descritor de um bloco de mem�ria global
 */
typedef struct queue_memoryDesc_s {
    void * initBlock;
    int size;
    int enqueue;
    int busy;
} queue_memoryDesc_t;

/** \brief Descritor da mem�ria global da queue
 */
typedef struct queue_memory_s {
    int qtMemory;
    void * memory_area[QUEUE_MAX_BLOCKS]; ///< ponteiro para a �rea de mem�ria alocada dinamicamente
    list<queue_memoryDesc_t *> free;      ///< lista para ponteiros de descritores de mem�ria livres
    list<queue_memoryDesc_t *> busy;      /** lista para ponteiros de descritores de peda�os de mem�ria
                                             ocupada */
    pthread_mutex_t queue_mutex;
    int blocks;
} queue_memory_t;

// funcoes privadas

/** \brief Libera todos os elementos da queue
 *  \internal
 */
void queue_flushInternal(queue_t *);

/** \brief Desregistra todos os consumidores da queue
 *  \internal
 */
void queue_unregisterAllConsumers(queue_t *);

/** \brief Aumenta o n�mero de consumidores sempre que um novo � inserido
 *  \internal
 */
void queue_incStoreConsumers(queue_store_t *);

/** \brief Reduz o n�mero de consumidores sempre que um � removido
 *  \internal
 */
void queue_decStoreConsumers(queue_store_t *);

/** \brief Atualiza os consumidores que j� consumiram todos os itens par avis�-los
 *         que um novo item foi inserido
 *  \internal
 */
void queue_updateConsumers(queue_t *,queue_store_t *);

/** \brief Libera um elemento da queue que j� foi usado por todos consumidores
 *  \internal
 */
void queue_freeDirect(queue_t *, queue_store_t *);

/** \brief Aloca um espa�o de mem�ria na mem�ria global da queue
 *  \internal
 */
void *queue_statAlloc(size_t size);

/** \brief Libera um espa�o de mem�ria da mem�ria global da queue
 *  \internal
 */
void queue_statFree(void *);

#endif // _QUEUE_INT_H_
