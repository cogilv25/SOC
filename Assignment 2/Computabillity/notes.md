# bubble sort
- may be faster when sorting an array in reverse order than Big O would predict due to the branch predictor being able to more correctly predict the execution flow due to the fact the branch will go in the same direction each time, this is a clear example of the serious flaws with Big O thinking pointed out by some figure heads in performance critical areas such as... (maybe make this point after pointing out multiple instances)

# merge sort
- random being worse. Likely due to branch prediction again, however some optimizations I made may have had an impact as well.

# insertion sort
- probably as expected