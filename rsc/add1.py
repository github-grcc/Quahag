import sys

def replace_ends(input_path, output_path=None):
    """
    将文本文件每一行第一个和最后一个字符替换为 '1'。
    - 空行保持不变。
    - 若行只有一个字符，则该字符变为 '1'。
    - 若不指定输出路径，则直接覆盖原文件。
    """
    if output_path is None:
        output_path = input_path

    with open(input_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    with open(output_path, 'w', encoding='utf-8') as f:
        for line in lines:
            # 去掉行尾换行符，保留实际内容
            stripped = line.rstrip('\n')
            if stripped == '':
                f.write('\n')          # 空行原样保留
            else:
                if len(stripped) == 1:
                    new_line = '1'
                else:
                    # 中间部分保持不变，首尾变为 '1'
                    new_line = '1' + stripped[1:-1] + '1'
                f.write(new_line + '\n')

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("用法: python script.py 输入文件 [输出文件]")
        sys.exit(1)

    in_file = sys.argv[1]
    out_file = sys.argv[2] if len(sys.argv) > 2 else None
    replace_ends(in_file, out_file)
