var line;
while(line = readline()) {
        var numbers = line.split(" ").map(function(x) { return parseInt(x); });
        print(numbers[0] + numbers[1]);
}

