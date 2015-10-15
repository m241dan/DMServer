echo "Resetting Frame Scripts"

for file in ../scripts/frames/*.lua
do
   if [ ! -z $file ]
   then
      rm $file
   fi
done

echo "Resetting Instance Scripts"

for file in ../scripts/instances/*.lua
do
   if [ ! -z $file ]
   then
      rm $file
   fi
done

echo "Resetting Stat Scripts"

for file in ../scripts/stats/*.lua
do
   if [ ! -z $file ]
   then
      rm $file
   fi
done
