# Resources for C++ Course: Sorting Algorithms in Quantitative Finance

This directory contains all the supplementary materials for the C++ course on sorting algorithms in quantitative finance.

## Directory Structure

- **Slides/**: Presentation materials and lecture notes
  - `Week1_Presentation.md`: Detailed transcript for Week 1 presentation on sorting algorithms
  - Additional presentations will be added for future weeks

- **Code_Examples/**: Sample code and demonstrations
  - `sorting_algorithms_manim.py`: Manim animation script for visualizing sorting algorithms
  - Additional code examples will be added for future weeks

## Using the Resources

### Slides

The slides directory contains detailed presentation transcripts that include:
- Lecture content
- Code snippets
- Visual references
- Discussion points

These can be used to follow along with the lectures or for review.

### Code Examples

The code examples directory contains:
- Complete, working code that demonstrates concepts from the lectures
- Animation scripts for visualizing algorithms
- Benchmark code for performance testing

## Setting Up the Environment

### For Manim Animations

To run the Manim animations, you'll need to set up a Python environment with Manim installed:

```bash
# Create a virtual environment
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install manim numpy

# Run an animation
manim -pqh Resources/Code_Examples/sorting_algorithms_manim.py SortingAlgorithm
```

### For C++ Code Examples

To compile and run the C++ examples, you'll need a C++ compiler (GCC, Clang, or MSVC):

```bash
# Compile a C++ example
g++ -std=c++17 -O3 Resources/Code_Examples/example.cpp -o example

# Run the compiled program
./example
```

## Contributing

If you'd like to contribute additional resources or improvements:

1. Fork the repository
2. Create a new branch for your changes
3. Submit a pull request with a clear description of your additions

## License

All materials in this repository are provided for educational purposes. Please respect the intellectual property rights and do not redistribute without permission. 