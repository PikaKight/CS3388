file = "Assignment 2/dog.txt"

def read_text():
    
    with open(file, 'r') as f:
        text = f.readlines()

    coordinates = text[0].split(' ')

    print(coordinates)

if __name__ == "__main__":
    read_text()