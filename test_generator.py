import random

num_vertices = input()  # Change this to the number of vertices you want
num_vertices = int(num_vertices)

infinity = 100000
starting = -1

# Create an empty adjacency matrix
adj_matrix = [[starting for _ in range(num_vertices)]
              for _ in range(num_vertices)]

for x in range(num_vertices):
    adj_matrix[x][x]=infinity

check=[]
for x in range(num_vertices):
    for y in range(num_vertices):
        if adj_matrix[x][y]!=starting :
           continue
        if(x == y):
            continue
        else :
            # print("in")
            number=random.randint(0,5000)
            while(1):
                  if number in check :
                    number=random.randint(0,5000)
                  else :
                    break
            r = random.random()
            check.append(number)
            if r < 0.25 :
                number=infinity

        
            adj_matrix[x][y]=number
            adj_matrix[y][x]=number
            
            


            # for _ in range(num_vertices*num_vertices):
            #     # Generate random numbers
            #     i = random.randint(0, num_vertices-1)
            #     j = random.randint(0, num_vertices-1)

            #     # Make sure that the edge is not a self loop
            #     if i == j:
            #         continue

            #     # Make sure that the edge is not already present
            #     r = random.random()

            #     if r < 0.75 and adj_matrix[i][j] == infinity:
            #         adj_matrix[i][j] = weight
            #         adj_matrix[j][i] = weight

            #     weight += random.randint(0,5)

            # open file in write mode with name ${num_vertices}.txt
with open("input"+str(num_vertices) + ".txt", "w") as f:
    f.write(str(num_vertices))
    f.write("\n")
    for i in range(num_vertices):
        for j in range(num_vertices):
            if adj_matrix[i][j] == infinity:
                f.write("100000 ")
            else:
                f.write(str(adj_matrix[i][j]) + " ")
        f.write("\n")
