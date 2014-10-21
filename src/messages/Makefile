CPP_PROTOFILES := $(patsubst %.proto,%.pb.cc,$(wildcard *.proto))
PYTHON_PROTOFILES := $(patsubst %.proto,../../berlinunited/install/protobuf/%_pb2.py,$(wildcard *.proto))
INC := -I. -I../../berlinunited/src/messages

all: $(CPP_PROTOFILES) 

python: $(PYTHON_PROTOFILES)

clean:
	rm -f $(wildcard *.pb.cc) $(wildcard *.pb.h) 

%.pb.cc: %.proto
	../../berlinunited/tools/protobuf/bin/protoc $(INC) --cpp_out=. $<

../../../berlinunited/install/protobuf/%_pb2.py: %.proto
	../../berlinunited/tools/protobuf/bin/protoc $(INC) --python_out=../../berlinunited/install/protobuf $<
