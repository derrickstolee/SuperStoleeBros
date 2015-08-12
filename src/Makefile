

OBJECTS =  	Actor.o 				\
			ActorFactory.o 			\
			BFS.o 					\
			dstolee_SmartEnemy.o 	\
			dstolee_SmartHero.o 	\
			dstolee_SmartPowerup.o 	\
			FloydWarshall.o 		\
			GameManager.o  			\
			GraphMap.o  			\
			Melee.o   				\
			OtherActors.o 			\
			Pursuer.o 				\
			SimpleHero.o


all : ssbros ssbrosmelee $(OBJECTS)

.SUFFIXES: .c .cpp .o 

ssbros : ssbros.cpp $(OBJECTS)
	g++ -g -Wall -o ssbros ssbros.cpp Actor.o 				\
			BFS.o 					\
			FloydWarshall.o 		\
			GameManager.o  			\
			GraphMap.o  			\
			OtherActors.o 			\
			SimpleHero.o -lncurses

ssbrosmelee : ssbrosmelee.cpp $(OBJECTS)
	g++ -g -Wall -o ssbrosmelee ssbrosmelee.cpp Actor.o 				\
			ActorFactory.o 			\
			BFS.o 					\
			dstolee_SmartEnemy.o 	\
			dstolee_SmartHero.o 	\
			dstolee_SmartPowerup.o 	\
			FloydWarshall.o 		\
			GameManager.o  			\
			GraphMap.o  			\
			Melee.o   				\
			OtherActors.o 			\
			SimpleHero.o -lncurses

# The default object compiler
.c.o: $<
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c $< -o $@
        
.cpp.o: $<
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LIBS) -c $< -o $@

# Modify the clean target to remove YOUR .o files...
clean:
	-rm ssbros $(OBJECTS)

tarball:
	-rm *.tar.gz
	tar czf project2.tar.gz Makefile GameManager.hpp GraphMap.hpp OtherActors.hpp Actor.hpp ssbros.cpp OtherActors.cpp *.o maps/*.txt
	tar czf project2-update.tar.gz Pursuer.hpp GameManager.hpp GraphMap.hpp OtherActors.hpp Actor.hpp OtherActors.cpp GameManager.o GraphMap.o Pursuer.o OtherActors.o Actor.o maps/*.txt
