/*
 * 实验一 · 任务 3：哲学家就餐与资源顺序分配（参考答案 · 函数版）
 *
 * 与 student/impl.c 结构完全相同，只是三处 TODO 已补完。
 * main 在 tests/harness.c 中，本文件不含 main。
 *
 * 自测：make test
 */

//测例提供的接口
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

        //TODO 1 的答案：资源顺序分配，永远先拿编号小的
        if (first > second) {
            int tmp = first;
            first   = second;
            second  = tmp;
        }

        //TODO 2 的答案：申请两把叉子并吃饭
        lab_fork_acquire(id, first);
        lab_fork_acquire(id, second);
        lab_meal(id);

        //TODO 3 的答案：按相反顺序释放
        lab_fork_release(id, second);
        lab_fork_release(id, first);
    }
}
