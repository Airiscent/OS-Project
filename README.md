# Authors

- John Whiting
- Connor McKinney
- Rob Kelly
- David Lewis
- Jack Troy

# Compiling the program

- `make` to build the program
- `make clean` to clean the program

# Usage

To run the program, use the following command after it has been built: `./a.out addresses.txt`

To change the parameters of the simulation, view the instructions below:
- Open `OSProject.c` in a text editor
- Adjust the `NUM_FRAMES` and `MEMORY_SIZE` macros to change the size of "physical memory"
    - Note that `MEMORY_SIZE` should be `256 * NUM_FRAMES`
