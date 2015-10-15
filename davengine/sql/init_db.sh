for file in *.sql
do
 echo "Importing $file" & mysql muddb -u m241dan -pGrc937! < $file
done
echo "Done, enjoy!"
