from PIL import Image
import argparse

def image_to_binary_text(image_path, output_width=100, threshold=128, output_file=None):
    """
    将 JPEG 图片转换为二值化字符画（由 '0' 和 '1' 组成）
    :param image_path: 输入图片路径
    :param output_width: 输出字符画的宽度（字符数），高度自动按比例缩放
    :param threshold: 二值化阈值（0-255），灰度值 < 阈值变为 '1'（黑），否则 '0'（白）
    :param output_file: 可选，保存结果的文本文件路径；为 None 时打印到控制台
    """
    # 1. 打开并转为灰度图
    img = Image.open(image_path).convert('L')

    # 2. 按比例缩放
    w, h = img.size
    aspect_ratio = h / w
    # 字符高度约占2倍宽度，乘以0.5以保持视觉比例
    output_height = int(aspect_ratio * output_width * 1)
    img = img.resize((output_width, output_height), Image.Resampling.LANCZOS)

    # 3. 获取像素数据
    pixels = img.getdata()

    # 4. 二值化映射
    chars = ['1' if p < threshold else '0' for p in pixels]

    # 5. 拼接为行
    lines = []
    for i in range(0, len(chars), output_width):
        line = ''.join(chars[i:i+output_width])
        lines.append(line)

    # 6. 构建最终文本：第一行为尺寸，再是字符画
    header = f"{output_width} {output_height}"
    text = header + '\n' + '\n'.join(lines)

    # 7. 输出
    if output_file:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(text)
        print(f"字符画已保存至 {output_file}")
    else:
        print(text)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='将 JPEG 图片转换为 0/1 二值化字符画，首行输出宽高')
    parser.add_argument('image', help='输入 JPEG 图片路径')
    parser.add_argument('-w', '--width', type=int, default=100, help='输出字符宽度，默认 100')
    parser.add_argument('-t', '--threshold', type=int, default=128, help='二值化阈值（0-255），默认 128')
    parser.add_argument('-o', '--output', help='保存到文本文件，不指定则直接打印')
    args = parser.parse_args()

    image_to_binary_text(args.image, args.width, args.threshold, args.output)
