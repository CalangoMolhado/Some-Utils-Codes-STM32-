import pygame
import serial
import random
from pygame.locals import *
# Abra a porta serial. Substitua 'COMx' pelo nome da sua porta serial.
com= input("Insira sua porta de comunicacao (ex:COM3): ").upper()
ser = serial.Serial(com, baudrate=115200, timeout=1)
x = 0
y = 0
z = 0
# Verifique se a porta serial está aberta
if ser.is_open:
    print(f"A porta serial {ser.port} está aberta.")

    # Inicializa o pygame
    pygame.init()

    # Configurações da janela
    width, height = 800, 600
    screen = pygame.display.set_mode((width, height))
    pygame.display.set_caption('Quadrado MPU650')

    # Cores
    BLACK = (0, 0, 0)
    CUBE_COLOR = (255, 0, 255)

    # Tamanho do cubo
    cube_size = 50

    ax = 0.5
    ay = 0.2

    # Posição inicial do cubo
    cube_x, cube_y = width // 2, height // 2

    borda = 0
    aux = 0
    # Velocidades iniciais
    vx, vy = 0, 0

    # Taxa de atualização (frames por segundo)
    fps = 60
    clock = pygame.time.Clock()

    running = True
    while running:
        try:
            # Leia uma linha de dados da porta serial
            linha = ser.readline().decode('utf-8').split(";")
            print(linha)
            ax = float(linha[0])
            ay = float(linha[1])
        except KeyboardInterrupt:
            pass

        for event in pygame.event.get():
            if event.type == QUIT:
                running = False

        # Atualiza a velocidade com base nos dados do sensor
        if 0.10 > float(ax) > -0.10 and 0.10 > float(ay) > -0.10:
            vx = vy = 0
        else:
            vx = float(ax) * -20
            vy = float(ay) * 20

        # Atualiza a posição do cubox
        cube_x += vx
        cube_y += vy
        # Limite para que o cubo não saia da tela
        if cube_x >= (width-cube_size//2) or cube_x < cube_size//2+1 or cube_y >= (height-cube_size//2) \
                or cube_y < cube_size//2+1:
            borda = 1
        else:
            borda = 0
            aux = 0
        if borda and aux == 0:
            aux = 1
            CUBE_COLOR = (random.randint(50, 255), random.randint(50, 255), random.randint(50, 255))

        cube_x = max(cube_size // 2, min(cube_x, width - cube_size // 2))
        cube_y = max(cube_size // 2, min(cube_y, height - cube_size // 2))

        # Limpa a tela
        screen.fill(BLACK)

        # Desenha o cubo na nova posição
        pygame.draw.rect(screen, CUBE_COLOR,
                         (cube_x - cube_size // 2, cube_y - cube_size // 2, cube_size, cube_size))

        # Atualiza a tela
        pygame.display.flip()

        # Limita a taxa de quadros por segundo
        clock.tick(fps)

    # Encerra o pygame
    pygame.quit()
    # Leia os dados da porta serial

    # Feche a porta serial quando terminar
    ser.close()
else:
    print(f"Não foi possível abrir a porta serial {ser.port}.")
