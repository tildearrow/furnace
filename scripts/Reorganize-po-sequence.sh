#!/bin/bash

merge_po_files_preserve_format() {
    base_file="base.po"
    new_file="new.po"
    output_file="output.po"

    declare -A new_translations

    # Read new_file and extract msgid and msgstr
    msgid=""
    while IFS= read -r line; do
        if [[ $line == msgid* ]]; then
            msgid="$line"
            msgstr=""
        elif [[ $line == msgstr* ]]; then
            msgstr="$line"
            new_translations["$msgid"]="$msgstr"
        elif [[ $line == "\"\"" ]]; then
            msgid+="$line"
        fi
    done < "$new_file"

    # Open output_file and write merged content
    msgid=""
    while IFS= read -r line; do
        if [[ $line == msgid* ]]; then
            msgid="$line"
            msgstr=""
        elif [[ $line == msgstr* ]]; then
            msgstr="$line"
            if [[ -n ${new_translations["$msgid"]} ]]; then
                echo "$msgid" >> "$output_file"
                echo "${new_translations["$msgid"]}" >> "$output_file"
            else
                echo "$msgid" >> "$output_file"
                echo "$msgstr" >> "$output_file"
            fi
            msgid=""
            msgstr=""
        elif [[ $line == "\"\"" ]]; then
            msgid+="$line"
        else
            echo "$line" >> "$output_file"
        fi
    done < "$base_file"
}

if [[ $# -ne 3 ]]; then
    echo "Usage: $0 base_file new_file output_file"
    exit 1
fi

merge_po_files_preserve_format "base.po" "new.po" "output.po"
