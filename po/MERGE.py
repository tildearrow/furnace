def merge_po_files_preserve_format(base_file, new_file, output_file):
    with open(base_file, 'r', encoding='utf-8') as f1, open(new_file, 'r', encoding='utf-8') as f2:
        base_lines = f1.readlines()
        new_lines = f2.readlines()

    # 遍历new_lines，提取msgid和msgstr
    new_translations = {}
    i = 0
    while i < len(new_lines):
        if new_lines[i].startswith('msgid '):
            msgid_start = i
            while not new_lines[i].startswith('msgstr '):
                i += 1
            msgid = ''.join(new_lines[msgid_start:i])
            msgstr = new_lines[i].split('msgstr ')[1].strip()
            new_translations[msgid] = msgstr
        i += 1

    # 打开output_file，写入合并后的内容
    with open(output_file, 'w', encoding='utf-8') as output:
        i = 0
        while i < len(base_lines):
            if base_lines[i].startswith('msgid '):
                msgid_start = i
                while not base_lines[i].startswith('msgstr '):
                    i += 1
                msgid = ''.join(base_lines[msgid_start:i])
                if msgid in new_translations:
                    output.write(f'{msgid}')
                    output.write(f'msgstr {new_translations[msgid]}\n')
                else:
                    output.write(base_lines[i])
                i += 1
            else:
                output.write(base_lines[i])
                i += 1

if __name__ == '__main__':
    base_file = 'base.po'
    new_file = 'new.po'
    output_file = 'output.po'
    merge_po_files_preserve_format(base_file, new_file, output_file)
