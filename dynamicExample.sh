# Make sure to disable BULKLOAD in the config file before executing this script
meson setup build
meson compile -C build
ln -s ./build/Index .
ln -s ./build/compile_commands.json .
./Index Example/sampleOperations.txt
