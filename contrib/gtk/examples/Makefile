OZC = ozc
OZCFLAGS = -z9

SRCS = HelloGTK.oz \
       HelloCanvas.oz \
       CanvasEvents.oz \
       CanvasMove.oz \
       CanvasScramble.oz

OBJS = $(SRCS:%.oz=%.ozf)

.PHONY: clean veryclean

all: $(OBJS)

%.ozf: %.oz
	$(OZC) $(OZCFLAGS) -c $< -o $@


clean:
	rm -f $(OBJS)

veryclean: clean
