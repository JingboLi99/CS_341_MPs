# tests stores argv as an array
tests=$@ 
# if argc is 0 run all tests
if [ $# -eq 0 ]
then
  tests=(fcfs sjf psrtf pri ppri rr)
fi

# Make main executable
make main

# Array variable to hold failed tests
fails=()
for scheme in ${tests[*]} 
do
  echo "Testing $scheme"
  echo "========================="
  # Run test case and diff with expected
  # timeout enforces that the program terminate within 30s
  timeout 30 ./main $scheme 2>/dev/null >/tmp/ss_output
  diff /tmp/ss_output examples/expected_$scheme.txt
  # If return status of diff is nonzero, add scheme to fails
  if [ ! $? -eq 0 ]
  then
    fails+=($scheme)
  fi
  echo "========================="
  echo ""
done
# Check length of fails for final results
if [ ${#fails[@]} -eq 0 ]
then
  echo "Looks good!"
else
  echo "Scheme(s) ${fails[@]} failed!"
fi

echo "Note that this tester only checks output generated every second, so passing this test does not guarantee passing the autograder!"
