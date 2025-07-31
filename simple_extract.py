#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

def extract_segment(input_file, output_file):
    """提取segment0到segment1之间的内容"""
    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 查找segment0和segment1的位置
        start_pos = content.find('segment 0')
        if start_pos == -1:
            print("未找到 'segment 0'")
            return False
        
        # 从segment0之后开始查找segment1
        search_start = start_pos + len('segment 0')
        end_pos = content.find('segment 1', search_start)
        if end_pos == -1:
            print("未找到 'segment 1'")
            return False
        
        # 提取内容（不包含segment0和segment1这两行）
        # 找到segment0行的结束位置
        start_line_end = content.find('\n', start_pos)
        if start_line_end == -1:
            start_line_end = len(content)
        
        # 提取segment0行之后到segment1行之前的内容
        extracted_content = content[start_line_end + 1:end_pos].strip()
        
        # 写入输出文件
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(extracted_content)
        
        print(f"成功提取内容到 {output_file}")
        return True
        
    except Exception as e:
        print(f"错误：{e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("用法：python simple_extract.py <输入文件> <输出文件>")
        sys.exit(1)
    
    extract_segment(sys.argv[1], sys.argv[2]) 