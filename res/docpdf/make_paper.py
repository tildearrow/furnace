#!/usr/bin/env python3

import os
import sys

import markdown
from mdx_gfm import GithubFlavoredMarkdownExtension

import weasyprint
import re

import logging
logging.basicConfig(format='%(levelname)s: %(message)s' ,stream=sys.stderr, level=logging.INFO)
LOGGER = logging.getLogger('preprocess')

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
      os.path.join(os.path.split(my_file)[0], match.group(2))
    )
  
  # urls to other files
  BASE_URL = 'https://github.com/tildearrow/furnace/tree/master/'
  if match.group(2).startswith(BASE_URL):
    file_path = match.group(2).split(BASE_URL)[-1]
    if os.path.splitext(file_path)[-1] == '':
      file_path += '/README.md'
    return '[%s](#%s)' % (
      match.group(1),
      file_path.replace('/','__')
    )
  
  # preserve external urls
  elif match.group(2).startswith('http'):
    return match.group(0)
  
  # fix paths
  act_path = os.path.split(my_file)[0] + '/' + match.group(2)
  act_path = os.path.relpath(os.path.abspath(act_path))
  return '[%s](#%s)' % (
    match.group(1),
    act_path.replace(os.path.sep,'__')
  )

def fix_headings(match):
  return '%s#' % (
    match.group(1)
  )

if __name__ == "__main__":
  #-- first, prepare the file list --#
  file_list = []
  for i in os.walk('../../doc'):
    base_dir, subfolders, files = i
    for file_ in filter(lambda x: x.lower().endswith('.md'), files):
      file_list.append(os.path.join(base_dir, file_))

  #-- then, create the index --#
  index = '<h2>contents</h2><ol>'

  #-- then, create the document --#
  html = ''

  # perform sort
  file_list.sort(key=sort_func)

  first = True

  for my_file in file_list:
    pageLink = my_file.replace(os.path.sep, "__")

    if pageLink.endswith("__doc__README.md"):
      continue

    with open(my_file, 'r') as md:
      LOGGER.info("processing file %s" % my_file)
      data = md.read()

    # retrieve title
    pageTitle = data.partition('\n')[0].replace("# ","")

    if pageLink.endswith("__README.md"):
      if first:
        first = False
      else:
        index += '</li></ol>'

    index += '<li><a href="#%s" class="indexItemPre">%s</a><a href="#%s" class="indexItem"></a>' % ( pageLink, pageTitle, pageLink )
    if pageLink.endswith("__README.md"):
      index += '<ol>'
    else:
      index += '</li>'
    
    # perform link fixing
    data = re.sub(r'\[(.+?)\]\((.+?)\)', fix_links, data)
    data = re.sub(r'^\s*(#+)', fix_headings, data, flags=re.MULTILINE)
    
    # each file is its own section
    html +='<section id="%s">%s</section>' % (
      my_file.replace(os.path.sep, "__"),
      markdown.markdown(data, extensions=['nl2br', 'mdx_breakless_lists', GithubFlavoredMarkdownExtension()])
    )

  # finish index
  index += '</ol></li></ol>'

  # build html
  final_html = ('''
    <!DOCTYPE html>
    <html lang="en">
      <head>
        <meta charset="utf-8"/>
        <title>Furnace Manual</title>
        <style>
          @font-face {
            font-family: 'Exo 2';
            font-style: normal;
            src: url("fonts/Exo2[wght].ttf") format("truetype");
          }
          @font-face {
            font-family: 'Exo 2';
            font-style: italic;
            src: url("fonts/Exo2-Italic[wght].ttf") format("truetype");
          }
          @font-face {
            font-family: 'IBM Plex Mono';
            font-style: normal;
            src: url("fonts/IBMPlexMono-Regular.ttf") format("truetype");
          }
          body {
            font-family: 'Exo 2';
            line-height: 1.25;
            font-size: 11pt;
            color: #000;
          }
          section {
            page-break-before:always;
            text-align: left;
            hyphens: auto;
          }
          code {
            font-family: 'IBM Plex Mono';
          }
          ul {
            padding-left: 10pt;
            margin-right: 4pt;
            list-style-type: none;
          }
          ul > li:before {
            content: '-';
            padding-right: 3pt;
          }
          h1 {
            hyphens: none;
            text-align: left;
          }
          h2 {
            hyphens: none;
            text-align: center;
            font-size: 36pt;
            margin: 0;
            padding: 0 0 4pt 10pt;
            border-style: none none double none;
            border-width: 0 0 6pt 0;
          }
          h3 {
            hyphens: none;
            text-align: left;
            font-size: 20pt;
            margin: 0;
            padding: 0 0 2pt 10pt;
            border-style: none none solid none;
            border-width: 0 0 2pt 0;
            border-color: #999;
          }
          h4 {
            hyphens: none;
            text-align: left;
            font-size: 15pt;
            margin: 0;
            padding: 0 0 2pt 4pt;
          }
          h5,h6 {
            hyphens: none;
            text-align: left;
          }
          img {
            max-width: 100%%;
          }
          a {
            color: #204788;
            text-decoration: none;
            letter-spacing: .01em;
          }
          a[href^='#']:after {
            content: target-counter(attr(href),page);
            font-weight: normal;
            font-size: 0.5em;
            color: #555;
          }
          a[href^='http']:after {
            content: ' (' attr(href) ') ';
            font-weight: normal;
            color: #555;
          }
          a.indexItemPre {
            color: #000;
            text-decoration: none;
            letter-spacing: .01em;
          }
          a.indexItemPre[href^='#']:after {
            content: ' ' leader('.') ' ';
            font-size: 1em;
          }
          a.indexItem {
            float: right;
            overflow: hidden;
          }
          a.indexItem[href^='#']:after {
            content: target-counter(attr(href),page);
            color: #000;
            font-size: 11pt;
          }
          #cover {
            height: 100%%;
            text-align: center;
            display: flex;
            flex-direction: column;
          }
          #cover * {
            flex-grow: 1;
          }
          #cover h1 {
            text-align: center;
            font-size: 2.25em;
          }
          pre {
            font-size: .8em;
          }
          li > p {
            display: inline;
          }
          table {
            display: block;
            width: 100%%;
            max-width: 100%%;
            overflow: auto;
            border-collapse: collapse;
            text-align: left;
          }
          table tr {
            border-top: 1pt solid #aaa;
          }
          th, td {
            padding: 2pt 3pt;
            border: 1pt solid #ccc;
          }
          th {
            hyphens: none;
            padding: 2pt 4pt;
            text-transform: uppercase;
            font-size: .8em;
          }
          td {
            font-size: 11pt;
          }
          @page {
            size: a4;
            margin: 14mm 16mm 3cm 16mm;
          }
          @page:left {
            @bottom-left {
              content: counter(page);
              font-family: 'Exo 2';
              font-size: 10pt;
            }
          }
          @page:right {
            @bottom-left {
              content: 'Furnace manual';
              font-family: 'Exo 2';
              font-size: 10pt;
              color: #777;
            }
            @bottom-right {
              content: counter(page);
              font-family: 'Exo 2';
              font-size: 10pt;
            }
          }
          @page:first {
            @bottom-left {
              content: '';
            }
            @bottom-right {
              content: '';
            }
          }
        </style>
      </head>
      <body>
        <section id="cover">
          <div>
          </div>
          <div>
            <img src="../logo.png" style="width: 288pt;">
            <h1>Furnace<br/>User Manual</h1>
          </div>
          <div>
            <i>for version 0.6.5</i>
          </div>
        </section>
        <section id="authors">
          <div>
          </div>
          <div>
            <h3>authors</h3>
            <ul>
              <li>brickblock369</li>
              <li>cam900</li>
              <li>DeMOSic</li>
              <li>Electric Keet</li>
              <li>freq-mod</li>
              <li>host12prog</li>
              <li>Lunathir</li>
              <li>tildearrow</li>
            </ul>
            <p>special thanks to ZoomTen for providing tools which assisted in the production of this document!</p>
            <p>copyright © 2024 tildearrow and other authors.</p>
            <p>this documentation is under the <a href="https://creativecommons.org/licenses/by/3.0/">Creative Commons Attribution 3.0 Unported</a> license.</p>
            <p>you may reproduce, modify and/or distribute this documentation provided this copyright notice (including license and attribution) is present and any necessary disclaimers whether modifications have been made.</p>
            <p>this documentation is provided as-is and without warranty of any kind.</p>
            <p>this manual is written for version 0.6.5 of Furnace.<br/>it may not necessarily apply to previous or future versions.</p>
        </section>
        <section id="index">
          %s
        </section>
        %s
      </body>
    </html>
    ''' % (
      index, html
    )
  )
  
  print(final_html)
