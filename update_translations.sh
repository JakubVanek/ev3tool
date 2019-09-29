#!/bin/bash
BASEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

function poname() {
  if [ "$1" == "libcomm" ]; then
    echo "libev3comm"
  elif [ "$1" == "libasm" ]; then
    echo "libev3asm"
  elif [ "$1" == "tool" ]; then
    echo "ev3tool"
  else
    echo "program"
  fi
}

function poversion() {
  echo "0.2.0"
}

for TARGET in libcomm libasm tool; do
  DIR="$BASEDIR/$TARGET"
  xgettext \
    --directory="$DIR/src" --output="$DIR/po/$(poname $TARGET).pot" \
    --c++ --keyword=_ --keyword=_n --keyword=_num:1,2 \
    --sort-by-file --add-location --from-code=utf-8 \
    --copyright-holder="Jakub Vaněk" --package-name="$(poname $TARGET)" --package-version="$(poversion $TARGET)" \
    --msgid-bugs-address="linuxtardis@gmail.com" \
    `find "$DIR/src" -name '*.cpp' -o -name '*.hpp'`

  for lang in $(cat "$DIR/po/LINGUAS"); do
    msgmerge \
      --directory="$DIR/src" \
      --update --sort-by-file --verbose --add-location \
      "$DIR/po/$lang/$(poname $TARGET).po" "$DIR/po/$(poname $TARGET).pot"
  done
done
