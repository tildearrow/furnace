#!/usr/bin/env python3

import os
import sys

import markdown
from mdx_gfm import GithubFlavoredMarkdownExtension

import re

import logging
logging.basicConfig(format='%(levelname)s: %(message)s' ,stream=sys.stderr, level=logging.INFO)
LOGGER = logging.getLogger('preprocess')

hosted = False

# sort the file order
def sort_func(x):
  # place "papers/" at the end (like an appendix)
  try:
    x.index('%sdoc%s' % (os.path.sep, os.path.sep))
  except ValueError:
    return 'z'
  
  # place readmes at the start of each section
  try:
    rm = x.index('README.md')
    return x[:rm] + '0'
  except ValueError:
    return x

# make the links work in-pdf
def fix_links(match):
  # images
  if os.path.splitext(match.group(2))[-1] == '.png':
    return '[%s](%s)' % (
      match.group(1),
      match.group(2)
    )

  # preserve external urls
  elif match.group(2).startswith('http'):
    return match.group(0)

  elif match.group(2).endswith('README.md'):
    return '[%s](%s)' % (
      match.group(1),
      match.group(2).replace('README.md','index.html')
    )
  
  # fix paths
  return '[%s](%s)' % (
    match.group(1),
    match.group(2).replace('.md','.html')
  )

def fix_headings(match):
  return '%s#' % (
    match.group(1)
  )

if __name__ == "__main__":
  # check whether hosted mode is on
  if len(sys.argv)>1:
    if sys.argv[1]=='hosted':
      hosted=True

  #-- first, prepare the file list --#
  file_list = []
  for i in os.walk('../../doc'):
    base_dir, subfolders, files = i
    for file_ in filter(lambda x: x.lower().endswith('.md'), files):
      file_list.append(os.path.join(base_dir, file_))

  #-- then, create the document --#
  html = ''

  # perform sort
  file_list.sort(key=sort_func)

  first = True

  for my_file in file_list:
    with open(my_file, 'r') as md:
      LOGGER.info("processing file %s" % my_file)
      data = md.read()

    # retrieve path
    pagePath = 'htmldoc' + os.path.sep + my_file[10:]
    pagePathH = re.sub(r'\.md$','.html',pagePath).replace("README.html","index.html")
    docDir = pagePath[:pagePath.rfind(os.path.sep)]
    LOGGER.info("path: %s" % pagePathH)

    if not os.path.exists(docDir):
      os.makedirs(docDir)

    # retrieve title
    pageTitle = data.partition('\n')[0].replace("# ","")
    
    # perform link fixing
    data = re.sub(r'\[(.+?)\]\((.+?)\)', fix_links, data)
    data = re.sub(r'^\s*(#+)', fix_headings, data, flags=re.MULTILINE)
    
    # convert
    html = '''
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <style>
      body {
        background-color: #222;
        color: #eee;
        font-family: sans-serif;
      }

      a {
        color: #3df;
      }

      a:visited {
        color: #fd3;
      }

      b {
        color: #fff;
      }

      h1 {
        text-align: center;
      }

      img {
        max-width: 100%%;
      }
    </style>
    <title>%s</title>
  </head>
  <body>
    %s
  </body>
</html>
''' % (
      pageTitle,
      markdown.markdown(data, extensions=['nl2br', 'mdx_breakless_lists', GithubFlavoredMarkdownExtension()])
    )

    with open(pagePathH, 'w') as ht:
      ht.write(html)
