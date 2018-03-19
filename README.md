
# Bounded Buffer Problem in C
-	Threadpool library is used to manage all of the thread.  
-	Users can press p to pause, r to resume the program or e to end the program. 
-	Each generator has a unique identifier which indicates its corresponding material. 
-	The material buffer was implemented using semaphores and mutexes to allow simultaneous access to the critical section as well as mutual exclusion for putting or taking materials from the material buffer. 
-	Generator for material 1 can produce for a maximum of 4 materials. 
-	Generator for material 2 can produce for a maximum of 3 materials. 
-	Generator for material 3 can produce for a maximum of 3 materials. 
-	The generators will halt once they filled up the material buffer. 
-	User must enter the number of operator who operates the generators. 
-	Each operator have a randomizer to determine which product to produce. 
-	Once the decision is made, the operator retrieve the materials and tools to construct the product. 
-	The tools are mutexes which ensured mutual exclusion of share resources. One tool can be hold by only one operator
-	After the product is completed the operator put the tools back and place the product onto the product buffer. 
-	A product is dropped if the latest product in the product buffer is the same or the difference between the number of the current product and each of the other 2 products is more than 10. 
-	The product buffer is implemented as a circular integer array of size 30. 
-	The product buffer represented the products A, B, C as 1, 2, and 3 for simplicity. 
