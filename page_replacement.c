#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INSTRUCTION_COUNT 320 
#define PAGE_SIZE 10           
#define PAGE_COUNT 32   
#define MIN_FRAMES 4
#define MAX_FRAMES 32

const double SEQUENTIAL_RATIOS[] = {0.25, 0.50, 0.75};
const int RATIO_COUNT = 3;
 * 生成指令地址序列
void generate_instruction_sequence(double sequential_ratio, int instructions[INSTRUCTION_COUNT]) {
    int count = 0;
    
    while (count < INSTRUCTION_COUNT) {
        int s = rand() % INSTRUCTION_COUNT;
        
        instructions[count++] = s;
        if (count >= INSTRUCTION_COUNT) break;
        
        int s1 = s + 1;
        if (s1 >= INSTRUCTION_COUNT) s1 = INSTRUCTION_COUNT - 1;
        instructions[count++] = s1;
        if (count >= INSTRUCTION_COUNT) break;
        
        double r = (double)rand() / RAND_MAX;
        
        if (r <= sequential_ratio) {
            int seq_count = 1 + rand() % 3;
            for (int i = 0; i < seq_count && count < INSTRUCTION_COUNT; i++) {
                int next = instructions[count-1] + 1;
                if (next >= INSTRUCTION_COUNT) next = INSTRUCTION_COUNT - 1;
                instructions[count++] = next;
            }
        } else {
            // 非顺序执行：前地址和后地址分布
            if (s > 0 && count < INSTRUCTION_COUNT) {
                int m = rand() % (s + 1);
                instructions[count++] = m;
                if (count >= INSTRUCTION_COUNT) break;
                
                // 顺序执行m+1
                int m1 = m + 1;
                if (m1 >= INSTRUCTION_COUNT) m1 = INSTRUCTION_COUNT - 1;
                instructions[count++] = m1;
                if (count >= INSTRUCTION_COUNT) break;
                
                int low = m + 2;
                if (low < INSTRUCTION_COUNT) {
                    int s2 = low + rand() % (INSTRUCTION_COUNT - low);
                    s = s2;
                }
            }
        }
    }
}
 * 将指令地址转换为页号
void convert_to_pages(int instructions[INSTRUCTION_COUNT], int pages[INSTRUCTION_COUNT]) {
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        pages[i] = instructions[i] / PAGE_SIZE;
    }
}

 * OPT页面置换算法
int opt_algorithm(int pages[INSTRUCTION_COUNT], int frame_num) {
    int mem[frame_num];
    int page_faults = 0;
    
    // 初始化内存为-1表示空闲
    for (int i = 0; i < frame_num; i++) {
        mem[i] = -1;
    }
    
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        int current_page = pages[i];
        int hit = 0;
        
        // 检查是否在内存中
        for (int j = 0; j < frame_num; j++) {
            if (mem[j] == current_page) {
                hit = 1;
                break;
            }
        }
        
        if (hit) continue;
        
        // 缺页处理
        page_faults++;
        
        // 查找是否有空闲位置
        int empty_pos = -1;
        for (int j = 0; j < frame_num; j++) {
            if (mem[j] == -1) {
                empty_pos = j;
                break;
            }
        }
        
        if (empty_pos != -1) {
            mem[empty_pos] = current_page;
            continue;
        }
        
        // 无空闲位置，选择最远将来才使用的页面替换
        int farthest = -1;
        int replace_pos = 0;
        
        for (int j = 0; j < frame_num; j++) {
            int next_use = INSTRUCTION_COUNT;  // 默认未来不再使用
            
            for (int k = i + 1; k < INSTRUCTION_COUNT; k++) {
                if (pages[k] == mem[j]) {
                    next_use = k;
                    break;
                }
            }
            
            if (next_use > farthest) {
                farthest = next_use;
                replace_pos = j;
            }
        }
        
        mem[replace_pos] = current_page;
    }
    
    return page_faults;
}

 * FIFO页面置换算法(先进先出)
int fifo_algorithm(int pages[INSTRUCTION_COUNT], int frame_num) {
    int mem[frame_num];
    int page_faults = 0;
    int fifo_index = 0;
    
    // 初始化内存为-1表示空闲
    for (int i = 0; i < frame_num; i++) {
        mem[i] = -1;
    }
    
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        int current_page = pages[i];
        int hit = 0;
        
        // 检查是否在内存中
        for (int j = 0; j < frame_num; j++) {
            if (mem[j] == current_page) {
                hit = 1;
                break;
            }
        }
        
        if (hit) continue;
        
        // 缺页处理
        page_faults++;
        
        // 查找是否有空闲位置
        int empty_pos = -1;
        for (int j = 0; j < frame_num; j++) {
            if (mem[j] == -1) {
                empty_pos = j;
                break;
            }
        }
        
        if (empty_pos != -1) {
            mem[empty_pos] = current_page;
        } else {
            // 先进先出替换
            mem[fifo_index % frame_num] = current_page;
            fifo_index++;
        }
    }
    
    return page_faults;
}

 * LRU页面置换算法(最近最久未使用)
int lru_algorithm(int pages[INSTRUCTION_COUNT], int frame_num) {
    int mem[frame_num];
    int last_use[frame_num];  // 记录最近使用时间
    int page_faults = 0;
    int time = 0;
    
    // 初始化内存为-1表示空闲
    for (int i = 0; i < frame_num; i++) {
        mem[i] = -1;
        last_use[i] = -1;
    }
    
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        int current_page = pages[i];
        int hit_pos = -1;
        
        // 检查是否在内存中
        for (int j = 0; j < frame_num; j++) {
            if (mem[j] == current_page) {
                hit_pos = j;
                break;
            }
        }
        
        if (hit_pos != -1) {
            // 命中，更新使用时间
            last_use[hit_pos] = time++;
            continue;
        }
        
        // 缺页处理
        page_faults++;
        
        // 查找是否有空闲位置
        int empty_pos = -1;
        for (int j = 0; j < frame_num; j++) {
            if (mem[j] == -1) {
                empty_pos = j;
                break;
            }
        }
        
        if (empty_pos != -1) {
            mem[empty_pos] = current_page;
            last_use[empty_pos] = time++;
            continue;
        }
        
        // 无空闲位置，选择最近最久未使用的页面替换
        int oldest = 0;
        for (int j = 1; j < frame_num; j++) {
            if (last_use[j] < last_use[oldest]) {
                oldest = j;
            }
        }
        
        mem[oldest] = current_page;
        last_use[oldest] = time++;
    }
    
    return page_faults;
}

 * 打印命中率表格
void print_results_table(double results[RATIO_COUNT][MAX_FRAMES + 1][3]) {
    for (int r = 0; r < RATIO_COUNT; r++) {
        double ratio = SEQUENTIAL_RATIOS[r];
        printf("\n");
        printf("================================================================================\n");
        printf("                        顺序执行比例: %3.0f%%\n", ratio * 100);
        printf("================================================================================\n");
        printf("内存容量\tOPT命中率\tFIFO命中率\tLRU命中率\n");
        printf("--------------------------------------------------------------------------------\n");
        
        for (int f = MIN_FRAMES; f <= MAX_FRAMES; f++) {
            printf("  %2d\t\t  %.4f\t\t  %.4f\t\t  %.4f\n",
                   f, results[r][f][0], results[r][f][1], results[r][f][2]);
        }
        printf("================================================================================\n");
    }
}

int main() {
    printf("\n");
    printf("================================================================================\n");
    printf("           页式虚拟存储管理 - 页面置换算法实验\n");
    printf("                   三种算法: OPT | FIFO | LRU\n");
    printf("================================================================================\n");
    
    srand(time(NULL));
    
    // 初始化结果数组
    double results[RATIO_COUNT][MAX_FRAMES + 1][3];
    memset(results, 0, sizeof(results));
    
    int instructions[INSTRUCTION_COUNT];
    int pages[INSTRUCTION_COUNT];
    
    // 对每个顺序比例进行测试
    for (int r = 0; r < RATIO_COUNT; r++) {
        double ratio = SEQUENTIAL_RATIOS[r];
        printf("\n>>> 正在生成指令序列(顺序比例: %.0f%%)...\n", ratio * 100);
        
        // 生成指令序列并转换为页号
        generate_instruction_sequence(ratio, instructions);
        convert_to_pages(instructions, pages);
        
        printf(">>> 测试内存容量范围: %d ~ %d 页框\n", MIN_FRAMES, MAX_FRAMES);
        
        // 对所有内存容量进行测试
        for (int f = MIN_FRAMES; f <= MAX_FRAMES; f++) {
            // OPT算法
            int faults_opt = opt_algorithm(pages, f);
            results[r][f][0] = 1.0 - (double)faults_opt / INSTRUCTION_COUNT;
            
            // FIFO算法
            int faults_fifo = fifo_algorithm(pages, f);
            results[r][f][1] = 1.0 - (double)faults_fifo / INSTRUCTION_COUNT;
            
            // LRU算法
            int faults_lru = lru_algorithm(pages, f);
            results[r][f][2] = 1.0 - (double)faults_lru / INSTRUCTION_COUNT;
        }
        
        printf(">>> 完成! (命中率数据已记录)\n");
    }
    
    print_results_table(results)；
    printf("\n实验完成！\n\n");
    
    return 0;
}