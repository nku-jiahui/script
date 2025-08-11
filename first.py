def func(input_file, output_file):
    with open(input_file, 'r', encoding='utf-8') as fin, \
         open(output_file, 'w', encoding='utf-8') as fout:

        found_segment_0 = False
        for line in fin:
            stripped = line.strip()
            if 'segment 1' in stripped:
                break  # 遇到 segment 1 就停止
            if found_segment_0:
                fout.write(line)  # 写入内容
            if 'segment 0' in stripped:
                found_segment_0 = True

        if not found_segment_0:
            raise ValueError(f"'segment 0' not found in {input_file}")

    print(f"Extraction completed: {output_file}")
if __name__ == "__main__":
 input_file = 'input.txt'
 output_file = 'output.txt'
 func(input_file, output_file)
 