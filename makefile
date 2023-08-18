all: phase1 phase2 phase3 phase4 phase5

phase1:
	@g++ client-phase1.cpp -o client-phase1

phase2:
	@g++ client-phase2.cpp -o client-phase2

phase3:
	@g++ client-phase3.cpp -o client-phase3 -lssl

phase4:
	@g++ client-phase4.cpp -o client-phase4

phase5:
	@g++ client-phase5.cpp -o client-phase5