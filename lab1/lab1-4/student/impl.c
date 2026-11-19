/*
 * 实验一 · 任务 4：生产者与消费者（学生提交文件）
 *
 * 这个文件由 tests/harness.c 链接使用：main 在测例那边，你需要只写下面三个函数。
 *
 * 场景：
 *   5个生产者、5个消费者，共享一个容量为8的环形缓冲池。
 *   每个生产者生产4个产品，一共20个。
 *   产品用4位编号：最高位是生产者编号，其余是该生产者的产品序号。
 *   缓冲池满了，生产者要等；缓冲池空了，消费者要等。
 *
 * 测例提供两个缓冲读写原语，它们是故意不加锁的：
 *   void lab_buf_push(int product); 写入（调用前必须确认池内有空位）
 *   int  lab_buf_pop(void); 读取（调用前必须确认池内非空）
 *
 * 需要完成：
 *   TODO 1  pc_init 初始化两个信号量
 *   TODO 2  pc_put 放产品：等空位、加锁、写入、解锁、通知非空
 *   TODO 3  pc_get 取产品：等非空、加锁、读取、解锁、通知空位
 *
 * 提示：
 *   - 空位信号量的初值是缓冲池容量 bufsize，非空信号量的初值是 0；
 *   - 两个信号量只管“能不能进”，互斥要另外用 mutex 保证；
 *   - 信号量的等待（sem_wait）放在加锁之前，否则会死锁；
 *   - sem_post 放在解锁之后即可。
 *
 * 自测：make test-student
 */

#include <pthread.h>
#include <semaphore.h>

//看这边，同步对象已经声明好了，直接用就行
static sem_t empty_sem; //空位数量
static sem_t full_sem; //已放入的产品数量
static pthread_mutex_t buf_mtx = PTHREAD_MUTEX_INITIALIZER;

//测例提供的接口，别改，不然就出bug了
void lab_buf_push(int product);
int  lab_buf_pop(void);

/* ============================================================
 * TODO 1：初始化两个信号量
 *   sem_init(&empty_sem, 0, bufsize);
 *   sem_init(&full_sem,  0, 0);
 *   第二个参数0表示线程间共享
 * ============================================================ */
void pc_init(int bufsize)
{
    (void)bufsize;

    //TODO: 初始化empty_sem与full_sem
}

/* ============================================================
 * TODO 2：放入一个产品
 *   顺序很重要：先等空位，再加锁，然后写入，最后解锁并通知非空。写成下面这样：
 *   1. sem_wait(&empty_sem);           // 池满了就在这里等
 *   2. pthread_mutex_lock(&buf_mtx);
 *   3. lab_buf_push(product);          // 测例提供的写入
 *   4. pthread_mutex_unlock(&buf_mtx);
 *   5. sem_post(&full_sem);            // 告诉消费者有东西了
 * ============================================================ */
void pc_put(int product)
{
    (void)product;

    //TODO: 按上面的顺序实现
}

/* ============================================================
 * TODO 3：取出一个产品
 *   与 pc_put 对称：先等非空，再加锁，然后读取，这样写：
 *   1. sem_wait(&full_sem); // 池空了就在这里等
 *   2. pthread_mutex_lock(&buf_mtx);
 *   3. int v = lab_buf_pop(); // 测例提供的读取
 *   4. pthread_mutex_unlock(&buf_mtx);
 *   5. sem_post(&empty_sem); // 告诉生产者有空位了
 *   6. return v;
 * ============================================================ */
int pc_get(void)
{
    //TODO: 按上面的顺序实现

    return 0;
}
