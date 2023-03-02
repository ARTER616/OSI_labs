#!/bin/sh
interrupts=0 # счетчик количества прерываний

func(){ # обработчик прерываний
  interrupts=`expr $interrupts + 1` # увеличение счетчика прерываний
  k=`expr $interrupts % 5` # остаток от деления на 5
  echo
  echo "Interruption number: $interrupts" # вывод номера прерывания
  echo
  if [ $k -eq 0 ]; then # если остаток от деления равен 0, выводим общее количество блоков
    s=`du -s | awk '{print $1}'` # получаем общее количество блоков
    echo "Total blocks: $s"
  fi
}
trap func INT # замена обработчика прерываний

du_output=`du -c *` # записываем вывод du -c в переменную

files=`echo "$du_output" | awk '$1 > 4 {print $2}'` # awk сравнивает первый операнд (кол-во блоков) с 4, выводит большие 4
echo "Files with block count greater than 4:"

for i in $files
do
  if [ "$i" = "total" ]; then # завершаем цикл при встрече total
    break
  fi
  echo $i # выводим имена
  sleep 1 # спим секунду
done