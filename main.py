import os
import sais
import manber_myers

# Rutas libros
littleWomen = r"libros/Little_Women.txt"
metamorphosis = r"libros/Metamorphosis.txt"
pridePrejudice = r"libros/Pride_and_Prejudice.txt"
phantomOpera = r"libros/The_Phantom_of_the_Opera.txt"

libros = [metamorphosis, littleWomen, pridePrejudice, phantomOpera]

if __name__ == "__main__":
    #print(" ==== SAIS ====")

    #for libro in libros:
       # sais.procesar_libro(libro)

    print("Manber and Myers")

    for libro in libros:
        print(f"{libro}")
        manber_myers.mediciones(libro)


