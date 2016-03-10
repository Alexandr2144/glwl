## Что это?

Простой проект фреймворка 3D графики на чистом OpenGL и WinAPI. Основным критерием при
разработке была модульность, гибкость и независимость модулей. Имеющаяся основа даёт возможность
легко настраивать каждый модулей в отдельности, его составляющие независимы и позволяют менять их код, 
не влезая во внутренности модуля, можно написать свой компонент. Такая система позволяет
настраивать возможности каждого объекта: будет ли он выбрасывать исключения или вообще проверять ошибки,
стоит ли завернуть указатель glMapBuffer в умную обёртку или оставить так, нужен ли буфферизованный ввод/вывод
в видеопамять и многое другое.

Из того, что есть: простой вывод текста средствами WinGDI; ввод/вывод, возможность вызвать callback, при наступлении
события; работа с видеопамятью, её буфферизация, отображение в память; политики обработки
исключений и ошибок; VBO, IBO, UBO, VAO; свет, камера, материалы; текстуры, skybox; простое управление
шейдерами; пока один-единственный примитив — куб (не в смысле "только его и сможет вывести").

Первые в очереди на реализацию: framebuffer (уже есть в виде FXAA фильтра, осталось привести в нормальный вид);
ландшафт, хранение, перемещение по нему; kd-деревья; удобное хранение и работа с текстурами (возможность задания поведения
текстурных координат для анимированных спрайтов, например, и хранение и передача сразу нескольких текстур).

## Примеры работы
Всего реализованы три демки: один тест освещения, тест текстур и шейдеров.
### Тест 1
Первая на сцене рисует две лампы (синяя и белая), куб и [губку Менгера](https://ru.wikipedia.org/wiki/Губка_Менгера).
Заодно и тест производительности вышел. Губка имеет примерно 24^n вершин, где n — уровень рекурсии.
Для куба — нуль, на первом скриншоте — три. Модель освещения Кука-Торренса с упрощённым коэффициентом Френеля.

> Управление:
> > Камера: WASD — Мышь
> > Первый источник света: Up-Left-Right-Down
> > > Второй источник света: Num_8-Num_4-Num_2-Num_6

> > > Вверх-Вниз: Num_9-Num_3

Скриншоты:
![alt text](https://github.com/Alexandr2144/glwl/blob/master/Screenshots/3.jpg)
![alt text](https://github.com/Alexandr2144/glwl/blob/master/Screenshots/4.jpg)
![alt text](https://github.com/Alexandr2144/glwl/blob/master/Screenshots/5.jpg)

### Тест 2
Вторая сцена имеет один ящик и один "стеклянный" кубик. Вдобавок, всё это в skybox-е.
Управление такое же.

Скриншоты:
![alt text](https://github.com/Alexandr2144/glwl/blob/master/Screenshots/6.jpg)
![alt text](https://github.com/Alexandr2144/glwl/blob/master/Screenshots/7.jpg)

### Тест 3
В третьем тесте проверялись шейдеры и ray-casting. Объект на картинке — [Mandelbulb](https://en.wikipedia.org/wiki/Mandelbulb).
Взята была Quintic formula с коэффициентами A, B, D = 0 и C = 1. Освеение уже выглядит поинтереснее. С фильтрацией возникали сложности:
то слишком размыто, то всё равно не достаточно, то эффекта нет вообще. В дальнейшем думаю применить полный ray-tracing, но для
этого шейдеров будет маловато.

Скриншоты:
![alt text](https://github.com/Alexandr2144/glwl/blob/master/Screenshots/0.jpg)
![alt text](https://github.com/Alexandr2144/glwl/blob/master/Screenshots/1.jpg)
![alt text](https://github.com/Alexandr2144/glwl/blob/master/Screenshots/2.jpg)

##Как устроено
Основой архитектуры является модуль buf::basic_raw, который представляет из себя просто обёртку над
буффером OpenGL. Поведение buf::basic_raw можно настраивать за счёт политик. Всего у него 3 политики:
ExceptionPolicy, MutablePolicy и MapPolicy. Первая отвечает за проверку и генерацию исключительных ситуаций.
Примерный вид даного класса
```
class ExceptionPolicyInstance {
	public:
		enum code { sucess };

		GLenum glerr();
		code error();
		code exception(code mask);
		code exception();
	protected:
		template <typename FTy, typename... ArgsTy>
		inline void call(FTy f, ArgsTy... args) const;
		template <typename... MsgTypes>
		inline void check(bool err, MsgTypes... msg) const;

		/*...*/
	};
```
Public методы позвояют получить текущую ощибку и настроить фильтр исключений. Они могут быть любыми. Protected
методы обязательны для реализации. Первый вызывает функцию с заданными параметрами и проверяет результат. Вторая
сообщает об ошибке, если err = true.

MutablePolicy позволяет создать изменяемый или неизменяемый буффер.
```
template <policy ExceptionPolicy>
class MutablePolicyInstance : /*...*/ {
public:
	inline void init(GLenum type, GLbitfield flags, GLuint size, const void* data);
protected:
	/*...*/
};
```
Функция init создаёт буффер.

MapPolicy определяет тип управления указателем на отображённую память.
```
template <
		policy ExceptionPolicy,
		policy MutablePolicy
> struct MapPolicyInstance : /*...*/ {
public:
	typedef void* mapptr;

	inline void unmap(GLenum type);
	inline mapptr map(GLenum type, GLenum acess = READ_WRITE, 
						GLsizeiptr length = MAX, GLintptr offset = 0);
protected:
	/*...*/
};
```
Сам класс basic_raw (упрощённо):
```
template <
	class ExceptionPolicy,
	class MutablePolicy,
	class MapPolicy
> class basic_raw : public /*...*/ {
private:
	typedef ExceptionPolicy checker;
	typedef const basic_raw& cref;
public:
	typedef typename checker::code errcode;

	void bind(GLenum type, GLuint index = 0, GLintptr offset = 0, GLsizeiptr length = MAX) const;

	void read(GLenum type, GLuint offset, GLuint size, void* data);
	void write(GLenum type, GLuint offset, GLuint size, const void* data);
	void copy(GLuint offset, cref src, GLuint src_offset, GLuint write_len);

	GLuint id() const;

	/*...*/
};
```
И наследующий его и предоставляющий более полный интерфейс buf::raw
```
template <class ExceptionPolicy,
		class MutablePolicy,
		class MapPolicy>
class raw : protected /*...*/ {
private:
	typedef basic_raw<ExceptionPolicy, MutablePolicy, MapPolicy> base;
	typedef MutablePolicy<ExceptionPolicy> initp;
	typedef ExceptionPolicy checker;
	typedef const raw& cref;
public:
	typedef typename base::errcode errcode;
	typedef typename base::mapptr mapptr;

	raw(_STD _Uninitialized);
	raw(GLenum type, GLenum usage, GLuint size = 0, const void* data = nullptr);

	raw(raw&& src);
	raw(const raw& src);
	raw& operator=(raw&& src);
	raw& operator=(const raw& src);

	void bind(GLuint index = 0, GLintptr offset = 0, GLsizeiptr length = MAX) const;

	void unmap();
	typename base::mapptr map(GLenum acess);
	typename base::mapptr map(GLintptr offset, GLsizeiptr length = MAX, GLenum acess_ext);

	void read(GLuint offset, GLuint size, void* data);
	
	void write(GLuint offset, GLuint size, const void* data);
	
	void copy(GLuint offset, cref src, GLuint src_offset, GLuint write_len);

	void reserve(GLuint size, const void* data = nullptr);
	void resize(GLuint size, const void* data);

	GLenum type( [GLenum new_type] );
	GLenum usage( [GLenum new_usage] );
	GLuint capacity() const;
	
	/*...*/
};
```

###To be continued...
