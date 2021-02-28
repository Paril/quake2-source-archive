from glob import glob

env = Environment()

env.Append(CCFLAGS='-g -pthread')

sources = glob("*.c") + glob ("ai/*.c")

env.SharedLibrary('gamex86', source=sources, CPPPATH='/usr/include/mysql', LIBS='mysqlclient')
