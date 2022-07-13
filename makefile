all: SOKE cal rel maxgf opt sample_graph
SOKE: SOKE.o
	g++ -o SOKE SOKE.o -pthread
SOKE.o: SOKE.cpp Graph.h
	g++ -c SOKE.cpp -std=c++11 -pthread
opt: opt.o
	g++ -o opt opt.o -pthread
opt.o: opt.cpp Graph.h
	g++ -c opt.cpp -std=c++11 -pthread
sample_graph: sample_graph.o
	g++ -o sample_graph sample_graph.o -pthread
sample_graph.o: sample_graph.cpp Graph.h
	g++ -c sample_graph.cpp -std=c++11 -pthread
cal: cal.o
	g++ -o cal cal.o -pthread
cal.o: cal.cpp Graph.h
	g++ -c cal.cpp -std=c++11 -pthread
rel: rel.o
	g++ -o rel rel.o -pthread
rel.o: rel.cpp Graph.h
	g++ -c rel.cpp -std=c++11 -pthread
combine: combine.o
	g++ -o combine combine.o -pthread
combine.o: combine.cpp
	g++ -c combine.cpp -std=c++11 -pthread
maxgf: maxgf.o
	g++ -o maxgf maxgf.o -pthread
maxgf.o: maxgf.cpp Graph.h
	g++ -c maxgf.cpp -std=c++11 -pthread
clean:
	rm -f *.o *.exe
