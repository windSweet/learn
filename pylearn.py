a = 1
print(id(a))
try:
    print(b) # type: ignore
except:
    print(a)