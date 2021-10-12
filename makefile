
coordinator: 
	gcc  -I ./Headers ./Source/coordinator.c -pthread -lm -o ./build/coordinator
	
		
clean:
	rm -f ./build/coordinator