OZC = ozc
OZCFLAGS = -z9

SRCS = HelloGTK.oz \
       HelloText.oz \
       HelloCanvas.oz \
       HelloArgs.oz \
       CanvasEvents.oz \
       CanvasMove.oz \
       CanvasScramble.oz \
       List.oz \
       FileSelection.oz

OBJS = $(SRCS:%.oz=%.ozf)

.PHONY: clean veryclean

all: $(OBJS)

%.ozf: %.oz
        $(OZC) $(OZCFLAGS) -c $< -o $@

clean:
        rm -f $(OBJS)

veryclean: clean
