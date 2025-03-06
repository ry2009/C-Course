#!/bin/bash

# Script to manage access to instructor-only files in the C++ course repository
# Usage: ./manage_access.sh [instructor|student]

MODE=$1

if [ "$MODE" == "instructor" ]; then
    echo "Setting up repository for instructor access (all files visible)"
    
    # Remove instructor-only patterns from .gitignore
    sed -i.bak '/# Instructor-only materials/,$d' .gitignore
    echo "# Instructor-only materials section removed from .gitignore"
    
    # Disable export-ignore attributes
    sed -i.bak '/# Instructor-only files/,$d' .gitattributes
    echo "# Instructor-only section removed from .gitattributes"
    
    echo "Repository now configured for instructor access."
    echo "All files will be visible and tracked."

elif [ "$MODE" == "student" ]; then
    echo "Setting up repository for student access (restricted files hidden)"
    
    # Check if instructor-only section already exists in .gitignore
    if ! grep -q "# Instructor-only materials" .gitignore; then
        # Add instructor-only patterns to .gitignore
        cat >> .gitignore << EOF

# Instructor-only materials
Resources/Slides/manim_transcript.md
Resources/Slides/Week1_Course_Plan.md
Resources/Code_Examples/sorting_algorithms_manim.py
Week2/
Week3/
*_solution.*
*_answers.*
*_key.*
EOF
        echo "# Instructor-only section added to .gitignore"
    fi
    
    # Check if instructor-only section already exists in .gitattributes
    if ! grep -q "# Instructor-only files" .gitattributes; then
        # Add instructor-only patterns to .gitattributes
        cat >> .gitattributes << EOF

# Instructor-only files (will be excluded from archives)
Resources/Slides/manim_transcript.md export-ignore
Resources/Slides/Week1_Course_Plan.md export-ignore
Resources/Code_Examples/sorting_algorithms_manim.py export-ignore
Week2/** export-ignore
Week3/** export-ignore
*_solution.* export-ignore
*_answers.* export-ignore
*_key.* export-ignore
EOF
        echo "# Instructor-only section added to .gitattributes"
    fi
    
    echo "Repository now configured for student access."
    echo "Instructor-only files will be hidden from students."
    
else
    echo "Error: Invalid mode specified."
    echo "Usage: ./manage_access.sh [instructor|student]"
    echo "  instructor: Makes all files visible (for your use)"
    echo "  student: Hides instructor-only files (for student access)"
    exit 1
fi

# Clean up backup files
rm -f .gitignore.bak .gitattributes.bak

echo "Done." 