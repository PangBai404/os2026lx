/*
 * 实验一 · 任务 3：哲学家就餐与资源顺序分配（学生提交文件）
 *
 * 这个文件由 tests/harness.c 链接使用：main 在测例那边，你只写 philosopher_run。
 *
 * 场景：
 *   5 位哲学家围坐一桌，每人左右各有一把叉子，一共 5 把。
 *   哲学家 id 的左右两把叉子编号分别是 id 和 (id + 1) % nphil。
 *   吃饭需要同时握着这两把叉子。
 *
 * 现在的写法是「先左后右」。5 位哲学家同时举左边的叉子时，
 * 每人都在等右边那把——这就是死锁。
 *
 * 需要完成：
 *   TODO 1  用资源顺序分配法决定先申请哪把叉子
 *   TODO 2  依次申请两把叉子，然后调用 lab_meal 吃饭
 *   TODO 3  依次释放两把叉子
 *
 * 提示：
 *   - 资源顺序分配法的要求：永远先申请编号小的那把；
 *   - 只需要在必要时交换 first 和 second，死锁的循环等待就被打破了；
 *   - lab_fork_acquire 在叉子被占用时会阻塞，不需要自己写等待逻辑；
 *   - 释放的顺序不影响死锁，但习惯上按申请的相反顺序释放。
 *
 * 自测：make test-student
 */

/* 测例提供的接口，签名固定，不要改动 */
int  lab_fork_acquire(int phid, int fork);
void lab_fork_release(int phid, int fork);
void lab_meal(int phid);

void philosopher_run(int id, int nphil, int rounds)
{
    for (int r = 0; r < rounds; r++) {
        int left  = id;
        int right = (id + 1) % nphil;

        int first  = left;
        int second = right;

        /* ============================================================
         * TODO 1：资源顺序分配
         *   永远先申请编号小的那把叉子。
         *   提示：比较 first 和 second，必要时交换。
         * ============================================================ */

        /* ============================================================
         * TODO 2：申请两把叉子并吃饭
         *   lab_fork_acquire(id, first);
         *   lab_fork_acquire(id, second);
         *   lab_meal(id);
         * ============================================================ */

        /* ============================================================
         * TODO 3：释放两把叉子
         *   lab_fork_release(id, second);
         *   lab_fork_release(id, first);
         * ============================================================ */

        (void)first;
        (void)second;
    }
}
