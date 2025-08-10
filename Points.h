#include <vector>
#include <array>
#include <numeric> // Для std::accumulate

// --- Подключаем GLM ---
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // Для translate, rotate, scale
#include <glm/gtc/type_ptr.hpp>         // Для glm::value_ptr

/*
 * Класс для управления 4 точками фигуры.
 * Хранит исходные точки и применяет к ним трансформации (перемещение, масштаб, поворот).
 */
class Points {
public:
    // Конструктор принимает массив из 8 float (4 точки xy)
    Points(const float initialPoints[8]) {
        // Копируем исходные точки в наш массив векторов
        for (int i = 0; i < 4; ++i) {
            m_originalPoints[i] = glm::vec2(initialPoints[i * 2], initialPoints[i * 2 + 1]);
        }
        calculateCenter();
        updateTransformedPoints(); // Первоначальный расчёт
    }

    // Задаёт перемещение фигуры
    void move(float x, float y) {
        m_translation = glm::vec2(x, y);
    }

    // Задаёт масштабирование фигуры относительно её центра
    void scale(float scaleX, float scaleY) {
        m_scale = glm::vec2(scaleX, scaleY);
    }

    // Задаёт поворот фигуры в градусах относительно её центра
    void rotate(float degrees) {
        m_rotationDegrees = degrees;
    }

    void reset() {
        move(0.f,0.f);
        scale(1.f,1.f);
        rotate(0.f);
        m_deform = {
            glm::vec4(0.f, 0.f, 0.f, 0.f),  // bottom-left
            glm::vec4(0.f, 0.f, 0.f, 0.f),  // bottom-right
            glm::vec4(0.f, 0.f, 0.f, 0.f),  // top-right
            glm::vec4(0.f, 0.f, 0.f, 0.f)   // top-left
        };
    }

    void deform(float top, float bottom, float left, float right) {
        m_deform = {
            glm::vec4(-bottom, -left, 0.f, 0.f),  // bottom-left
            glm::vec4(bottom, -right, 0.f, 0.f),  // bottom-right
            glm::vec4(top, right, 0.f, 0.f), // top-right
            glm::vec4(-top, left, 0.f, 0.f) // top-left
        };
    }

    // Возвращает указатель на массив float для передачи в glUniform2fv
    const float* getRawData() {
        updateTransformedPoints();
        return m_glData.data();
    }

private:
    // Рассчитывает и обновляет конечные координаты точек
    void updateTransformedPoints() {
        // 1. Создаём единичную матрицу
        glm::mat4 transform = glm::mat4(1.0f);

        // 2. Добавляем деформацию для каждой вершины
        std::array<glm::mat4, 4> deform_transforms = {
            glm::translate(glm::mat4(1.0f), glm::vec3(m_deform[0])),
            glm::translate(glm::mat4(1.0f), glm::vec3(m_deform[1])),
            glm::translate(glm::mat4(1.0f), glm::vec3(m_deform[2])),
            glm::translate(glm::mat4(1.0f), glm::vec3(m_deform[3]))
        };

        // 3. Основные преобразования
        transform = glm::translate(transform, glm::vec3(m_translation, 0.0f));
        transform = glm::translate(transform, glm::vec3(m_center, 0.0f));
        transform = glm::rotate(transform, glm::radians(m_rotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, glm::vec3(m_scale, 1.0f));
        transform = glm::translate(transform, glm::vec3(-m_center, 0.0f));

        // 4. Применяем преобразования и деформацию для каждой вершины
        for (int i = 0; i < 4; ++i) {
            // Комбинируем основное преобразование с индивидуальной деформацией
            glm::mat4 final_transform = transform * deform_transforms[i];
            
            // Применяем к исходной позиции вершины
            glm::vec4 transformed = final_transform * glm::vec4(m_originalPoints[i], 0.0f, 1.0f);
            
            m_glData[i * 2] = transformed.x;
            m_glData[i * 2 + 1] = transformed.y;
        }
    }

    // Вычисляет центр фигуры по исходным точкам
    void calculateCenter() {
        glm::vec2 sum(0.0f);
        for (const auto& p : m_originalPoints) {
            sum += p;
        }
        m_center = sum / 4.0f;
    }

    // --- Переменные ---
    std::array<glm::vec2, 4> m_originalPoints; // Исходные точки, не меняются
    std::array<float, 8> m_glData;             // Данные для OpenGL

    glm::vec2 m_center{0.0f, 0.0f};            // Центр фигуры
    glm::vec2 m_translation{0.0f, 0.0f};       // Вектор перемещения
    glm::vec2 m_scale{1.0f, 1.0f};             // Вектор масштаба
    // std::array<glm::vec2, 4> m_deform;      // хз, деформация
    std::array<glm::vec4, 4> m_deform;
    float m_rotationDegrees = 0.0f;            // Угол поворота
};