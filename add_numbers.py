#!/usr/bin/env python3
"""
给result.txt文件中的$符号后面添加连续编号，并清理变量名末尾的.+数字
"""

import sys
import os
import re

def clean_variable_name(var_name):
    """
    清理变量名末尾的.+数字模式
    例如: m_ptvCalcOffset1Ipat.2 -> m_ptvCalcOffset1Ipat
    """
    # 匹配末尾的 .数字 模式
    pattern = r'\.\d+$'
    return re.sub(pattern, '', var_name)

def add_numbers_to_result(input_file="/Users/jiahui/code/huawei-microcode-mapping/data/result.txt", output_file=None):
    """
    读取result.txt文件，给每个$后面添加连续编号，清理变量名，并去除重复映射
    
    Args:
        input_file: 输入文件路径
        output_file: 输出文件路径，如果为None则覆盖原文件
    """
    if not os.path.exists(input_file):
        print(f"错误：文件 {input_file} 不存在")
        return
    
    # 读取文件内容
    with open(input_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    # 处理每一行（按组缓冲，并在组内去重；若发生".数字$"连到下一个$，丢弃整组）
    counter = 1
    processed_lines = []

    current_group_started = False
    current_group_header = None  # 不含前缀"$ "的头部文本
    current_group_mappings = set()
    current_group_bad = False  # 是否命中了".数字$"导致整组删除

    def flush_group():
        nonlocal counter, processed_lines, current_group_started, current_group_header, current_group_mappings, current_group_bad
        if not current_group_started:
            return
        if not current_group_bad and len(current_group_mappings) > 0:
            processed_lines.append(f"${counter} " + current_group_header)
            for mapping in current_group_mappings:
                processed_lines.append(mapping)
            counter += 1
        # 重置组状态
        current_group_started = False
        current_group_header = None
        current_group_mappings = set()
        current_group_bad = False

    for raw_line in lines:
        line = raw_line.rstrip('\n\r')
        if line.startswith('$ '):
            # 新组开始：先冲刷上一组
            flush_group()
            current_group_started = True
            current_group_header = line[2:].strip()
            continue

        if not current_group_started:
            # 组外的其它行，原样保留
            processed_lines.append(line)
            continue

        # 处理组内行
        # 规则加强：只要该行内出现了符号"$"（且不是组头情况），视为粘连错误，整组删除
        if '$' in line:
            # 标记当前组为坏组（整组删除）
            current_group_bad = True
            # 将该行在第一个"$"处分割，左边丢弃（属于坏组），右边作为新组头
            try:
                left, right = line.split('$', 1)
            except ValueError:
                left, right = line, ''
            # 冲刷（删除）当前组
            flush_group()
            # 若存在新组头，开启新组
            if right:
                current_group_started = True
                current_group_header = right.strip()
            continue

        # 常规变量映射行
        if ' ~ ' in line:
            parts = line.split(' ~ ')
            if len(parts) == 2:
                left_var = clean_variable_name(parts[0].strip())
                right_var = clean_variable_name(parts[1].strip())
                mapping = f"{left_var} ~ {right_var}"
                if mapping not in current_group_mappings:
                    current_group_mappings.add(mapping)
            else:
                # 非标准行，保留在输出（组外逻辑留给上面处理，此处视为组内注释/空行）
                processed_lines.append(line)
        else:
            # 非映射行，直接原样保留
            processed_lines.append(line)

    # 文件结束，冲刷最后一组
    flush_group()
    
    # 确定输出文件
    if output_file is None:
        output_file = input_file
    
    # 写入处理后的内容
    with open(output_file, 'w', encoding='utf-8') as f:
        for line in processed_lines:
            f.write(line + '\n')
    
    print(f"处理完成！共处理了 {counter-1} 个匹配")
    print(f"输出文件：{output_file}")

def main():
    if len(sys.argv) > 1:
        input_file = sys.argv[1]
    else:
        input_file = "/Users/jiahui/code/huawei-microcode-mapping/data/result.txt"
    
    if len(sys.argv) > 2:
        output_file = sys.argv[2]
    else:
        output_file = None
    
    add_numbers_to_result(input_file, output_file)

if __name__ == "__main__":
    main()