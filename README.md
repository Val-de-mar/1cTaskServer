# 1cTaskServer

server принимает от 1 до 3х аргументов - <port> (-p) <output file>(-o) <online users limit>(-l)
	порт обязателен, все аргументы именованы(example: ./server -p 10000 -l 3 -o output.txt)
	по умолчанию количество пользователей - 5, а вывод идет в stdout

clien принимает 2 аргумента - <ip> <port> 
