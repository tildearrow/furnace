#  flac - Command-line FLAC encoder/decoder
#  Copyright (C) 2000-2009  Josh Coalson
#  Copyright (C) 2011-2025  Xiph.Org Foundation
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

if FLaC__HAS_PANDOC
flac.1: flac.md
	pandoc --standalone --to man $? > $@

metaflac.1: metaflac.md
	pandoc --standalone --to man $? > $@

flac.html: flac.md
	pandoc --standalone --to html --css style.css -B header.inc -A footer.inc --embed-resources $? > $@ || pandoc --self-contained --to html --css style.css -B header.inc -A footer.inc $? > $@

metaflac.html: metaflac.md
	pandoc --standalone --to html --css style.css -B header.inc -A footer.inc --embed-resources $? > $@ ||  pandoc --self-contained --to html --css style.css -B header.inc -A footer.inc $? > $@

man_MANS = flac.1 metaflac.1
doc_DATA = flac.html metaflac.html

else
if FLaC__HAS_PREBUILT_MANPAGES
man_MANS = flac.1 metaflac.1
endif
endif

EXTRA_DIST = flac.1 metaflac.1 flac.md metaflac.md flac.html metaflac.html
