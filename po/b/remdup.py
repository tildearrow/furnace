import sys

if len(sys.argv)<3:
  print("provide a file path and list of duplicates")
  exit()

with open(sys.argv[2]) as l:
  numbersS=l.read().splitlines()

numbers=[]

for i in numbersS:
  numbers.append(int(i))

with open(sys.argv[1]) as f:
  lines=f.read().splitlines()  

for i in numbers:
  curLine=i-1
  while lines[curLine]:
    lines[curLine]=str()
    curLine+=1

for i in lines:
  print(i)
