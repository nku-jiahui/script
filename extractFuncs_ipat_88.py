import os
import re

def is_start_line(line):
    return "_acc_start" in line

def is_end_line(line):
    return "_end" in line

def extract_function_prefix(line):
    trimmed = line.strip()
    if trimmed.startswith("-> "):
        trimmed = trimmed[3:]
    pos = trimmed.find("_acc_start")
    if pos != -1:
        return trimmed[:pos]
    return trimmed

def get_indent_level(line):
    return len(line) - len(line.lstrip(' \t'))

def set_indent(line, indent):
    return ' ' * indent + line.lstrip(' \t')

def generate_unique_filename(base_name, used_names):
    """生成唯一的文件名，如果名称已存在则添加编号"""
    if base_name not in used_names:
        used_names.add(base_name)
        return base_name
    
    # 如果名称已存在，添加编号
    counter = 1
    while f"{base_name}_{counter}" in used_names:
        counter += 1
    
    unique_name = f"{base_name}_{counter}"
    used_names.add(unique_name)
    return unique_name

input_file = input("请输入要分割的txt文件路径: ").strip()
output_dir = input("请输入输出文件夹路径: ").strip()
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

with open(input_file, 'r', encoding='utf-8') as f:
    lines = f.readlines()

i = 0
file_count = 0
used_names = set()  # 用于跟踪已使用的文件名

print("开始处理函数分割...")

while i < len(lines):
    if is_start_line(lines[i]):
        # 找end行
        end_pos = i
        for j in range(i + 1, len(lines)):
            if is_end_line(lines[j]):
                end_pos = j + 3  # C++逻辑：end后多输出几行
                break
        if end_pos > i:
            function_prefix = extract_function_prefix(lines[i])
            safe_name = re.sub(r'[\\/:*?"<>|\s]', '_', function_prefix)
            
            # 生成唯一文件名
            unique_name = generate_unique_filename(safe_name, used_names)
            out_path = os.path.join(output_dir, f"{unique_name}.txt")
            
            # 计算缩进调整量
            acc_start_indent = get_indent_level(lines[i])
            indent_delta = 4 - acc_start_indent
            
            with open(out_path, 'w', encoding='utf-8') as out_f:
                # 写头部
                out_f.write("Print Tree:\n")
                out_f.write("-> { (FUNCTION_DEF)  (BB:182014) (109, 431, 41, 41, 6)\n")
                out_f.write("  -> FUNCTION_LABEL (FUNCTION_LABEL)  (BB:182014) (109, 431, 41, 41, 6)\n")
                out_f.write(f"    -> {function_prefix} (IDENT)  (BB:4294967295) (109, 431, 45, 41, 6)\n")
                out_f.write("    -> ( ('(')  (BB:4294967295) (109, 431, 45, 41, 27)\n")
                out_f.write("    -> void (\"void\")  (BB:4294967295) (109, 431, 45, 41, 1)\n")
                # 写start前一行（inner label）
                if i > 0:
                    old_indent = get_indent_level(lines[i-1])
                    new_indent = max(0, old_indent + indent_delta)
                    out_f.write(set_indent(lines[i-1], new_indent) + "\n")
                # 写start到end的内容
                for k in range(i, min(end_pos+1, len(lines))):
                    old_indent = get_indent_level(lines[k])
                    new_indent = max(0, old_indent + indent_delta)
                    out_f.write(set_indent(lines[k], new_indent) + "\n")
            
            file_count += 1
            print(f"✓ 创建文件 {file_count}: {unique_name}.txt (原始名称: {function_prefix})")
            i = end_pos
        else:
            i += 1
    else:
        i += 1

print(f"\n处理完成，共创建 {file_count} 个文件，保存在 {output_dir} 下。")

# 显示重复名称统计
if len(used_names) < file_count:
    print(f"注意：发现 {file_count - len(used_names)} 个重复的函数名称，已自动添加编号区分。")