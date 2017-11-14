let i=1;
for file in tree*.txt ;
do
echo $file,out$file;
diff <(./hw1 $file) out$i.txt;
let i++;
done
