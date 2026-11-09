import os
import sais
import manber_myers
import maber_myers_modificado
import sais_modificado

# Rutas libros
littleWomen = r"libros/Little_Women.txt"
metamorphosis = r"libros/Metamorphosis.txt"
pridePrejudice = r"libros/Pride_and_Prejudice.txt"
phantomOpera = r"libros/The_Phantom_of_the_Opera.txt"

libros = [metamorphosis, littleWomen, pridePrejudice, phantomOpera]

if __name__ == "__main__":
    print(" ==== SAIS ====")

    for libro in libros:
       sais.procesar_libro(libro)
    
    print("\n ==== SAIS modificado ====")
    for libro in libros:
        sais_modificado.procesar_libro(libro)

    print(" ==== Manber and Myers ====")

    for libro in libros:
        print(f"{libro}")
        manber_myers.mediciones(libro)
        
    print("\n ==== Manber and Myers modificado ====")

    for libro in libros:
        print(f"\n{libro}")
        maber_myers_modificado.mediciones(libro)
        
    


