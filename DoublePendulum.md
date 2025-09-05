
The double pendulum is described by the masses $m_1$ and $m_2$ of the inner and outer pendulum, their lengths $\ell_1$ and $\ell_2$ and $g = 9.81$.

The movement is described by the angles $q_1$ and $q_2$ of both pendulums as well as the so called conjugated momentums $p_1$ and $p_2$.
They solve the following ordinary differential equation
$$ \dot q = \nabla_p H(q, p) \qquad \dot p = \nabla_q H(q, p) $$

where
$$ H(p, q) = \tfrac{1}{2} p^\top M(q)^{-1} p + U(q) $$
with
$$
    M(q)
    = \begin{pmatrix} (m_1+m_2)\ell_1^2 & m_2\ell_1\ell_2\cos(q_1-q_2) \\ m_2\ell_1 \ell_2 \cos(q_1-q_2)  & m_2\ell_2^2  \end{pmatrix}
$$
and
$$ U(q) = -(m_1 + m_2) g \ell_1 \cos(q_1) - m_2 g \ell_2 \cos(q_2) $$

To solve this differential equation we need
$$ \nabla_q H(p, q) = \begin{pmatrix}- g \ell_{1} \left(- m_{1} - m_{2}\right) \sin{\left(q_{1} \right)} + p_{1} \left(- \frac{\ell_{1}^{2} m_{2} p_{1} \sin{\left(q_{1} - q_{2} \right)} \cos{\left(q_{1} - q_{2} \right)}}{\left(\ell_{1}^{2} m_{1} - \ell_{1}^{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1}^{2} m_{2}\right)^{2}} + \frac{\ell_{1} \ell_{2} m_{2} p_{2} \sin{\left(q_{1} - q_{2} \right)} \cos^{2}{\left(q_{1} - q_{2} \right)}}{\left(\ell_{1} \ell_{2} m_{1} - \ell_{1} \ell_{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1} \ell_{2} m_{2}\right)^{2}} + \frac{p_{2} \sin{\left(q_{1} - q_{2} \right)}}{2 \left(\ell_{1} \ell_{2} m_{1} - \ell_{1} \ell_{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1} \ell_{2} m_{2}\right)}\right) + p_{2} \left(\frac{\ell_{1} \ell_{2} m_{2} p_{1} \sin{\left(q_{1} - q_{2} \right)} \cos^{2}{\left(q_{1} - q_{2} \right)}}{\left(\ell_{1} \ell_{2} m_{1} - \ell_{1} \ell_{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1} \ell_{2} m_{2}\right)^{2}} - \frac{\ell_{2}^{2} m_{2}^{2} p_{2} \left(m_{1} + m_{2}\right) \sin{\left(q_{1} - q_{2} \right)} \cos{\left(q_{1} - q_{2} \right)}}{\left(\ell_{2}^{2} m_{1} m_{2} - \ell_{2}^{2} m_{2}^{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{2}^{2} m_{2}^{2}\right)^{2}} + \frac{p_{1} \sin{\left(q_{1} - q_{2} \right)}}{2 \left(\ell_{1} \ell_{2} m_{1} - \ell_{1} \ell_{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1} \ell_{2} m_{2}\right)}\right)\\g \ell_{2} m_{2} \sin{\left(q_{2} \right)} + p_{1} \left(\frac{\ell_{1}^{2} m_{2} p_{1} \sin{\left(q_{1} - q_{2} \right)} \cos{\left(q_{1} - q_{2} \right)}}{\left(\ell_{1}^{2} m_{1} - \ell_{1}^{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1}^{2} m_{2}\right)^{2}} - \frac{\ell_{1} \ell_{2} m_{2} p_{2} \sin{\left(q_{1} - q_{2} \right)} \cos^{2}{\left(q_{1} - q_{2} \right)}}{\left(\ell_{1} \ell_{2} m_{1} - \ell_{1} \ell_{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1} \ell_{2} m_{2}\right)^{2}} - \frac{p_{2} \sin{\left(q_{1} - q_{2} \right)}}{2 \left(\ell_{1} \ell_{2} m_{1} - \ell_{1} \ell_{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1} \ell_{2} m_{2}\right)}\right) + p_{2} \left(- \frac{\ell_{1} \ell_{2} m_{2} p_{1} \sin{\left(q_{1} - q_{2} \right)} \cos^{2}{\left(q_{1} - q_{2} \right)}}{\left(\ell_{1} \ell_{2} m_{1} - \ell_{1} \ell_{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1} \ell_{2} m_{2}\right)^{2}} + \frac{\ell_{2}^{2} m_{2}^{2} p_{2} \left(m_{1} + m_{2}\right) \sin{\left(q_{1} - q_{2} \right)} \cos{\left(q_{1} - q_{2} \right)}}{\left(\ell_{2}^{2} m_{1} m_{2} - \ell_{2}^{2} m_{2}^{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{2}^{2} m_{2}^{2}\right)^{2}} - \frac{p_{1} \sin{\left(q_{1} - q_{2} \right)}}{2 \left(\ell_{1} \ell_{2} m_{1} - \ell_{1} \ell_{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1} \ell_{2} m_{2}\right)}\right)\end{pmatrix} $$

and

$$ \nabla_p H(p, q) = \begin{pmatrix}\frac{p_{1}}{\ell_{1}^{2} m_{1} - \ell_{1}^{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1}^{2} m_{2}} - \frac{p_{2} \cos{\left(q_{1} - q_{2} \right)}}{\ell_{1} \ell_{2} m_{1} - \ell_{1} \ell_{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1} \ell_{2} m_{2}}\\- \frac{p_{1} \cos{\left(q_{1} - q_{2} \right)}}{\ell_{1} \ell_{2} m_{1} - \ell_{1} \ell_{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1} \ell_{2} m_{2}} + \frac{p_{2} \left(m_{1} + m_{2}\right)}{\ell_{2}^{2} m_{1} m_{2} - \ell_{2}^{2} m_{2}^{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{2}^{2} m_{2}^{2}}\end{pmatrix} $$


### Equivalent differential equation
The following differential equation is equivalent, but not a hamiltonian system. I guess it being more simple makes it a better choice for numerical methods, that don't require a hamiltonian system, e.g. the explicit euler or `RK45`.
$$ \begin{pmatrix} \dot q \\ \dot v \end{pmatrix} = \begin{pmatrix} v \\ M^{-1}(q) f(q, v) \end{pmatrix} $$

where
$$ f(q, v)
= \ell_1 \ell_2 m_2 \sin(q_1 - q_2) \begin{pmatrix} -v_2^2 \\ v_1^2 \end{pmatrix} - \nabla U(q)
= \begin{pmatrix}- g \ell_{1} \left(m_{1} + m_{2}\right) \sin{\left(q_{1} \right)} - \ell_{1} \ell_{2} m_{2} v_{2}^{2} \sin{\left(q_{1} - q_{2} \right)}\\- g \ell_{2} m_{2} \sin{\left(q_{2} \right)} + \ell_{1} \ell_{2} m_{2} v_{1}^{2} \sin{\left(q_{1} - q_{2} \right)}\end{pmatrix} $$

Combined, we get
$$ \begin{pmatrix} \dot q \\ \dot v \end{pmatrix}
= \begin{pmatrix}v_{1}\\v_{2}\\\frac{- g \ell_{1} \left(m_{1} + m_{2}\right) \sin{\left(q_{1} \right)} - \ell_{1} \ell_{2} m_{2} v_{2}^{2} \sin{\left(q_{1} - q_{2} \right)}}{\ell_{1}^{2} m_{1} - \ell_{1}^{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1}^{2} m_{2}} - \frac{\left(- g \ell_{2} m_{2} \sin{\left(q_{2} \right)} + \ell_{1} \ell_{2} m_{2} v_{1}^{2} \sin{\left(q_{1} - q_{2} \right)}\right) \cos{\left(q_{1} - q_{2} \right)}}{\ell_{1} \ell_{2} m_{1} - \ell_{1} \ell_{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1} \ell_{2} m_{2}}\\\frac{\left(m_{1} + m_{2}\right) \left(- g \ell_{2} m_{2} \sin{\left(q_{2} \right)} + \ell_{1} \ell_{2} m_{2} v_{1}^{2} \sin{\left(q_{1} - q_{2} \right)}\right)}{\ell_{2}^{2} m_{1} m_{2} - \ell_{2}^{2} m_{2}^{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{2}^{2} m_{2}^{2}} - \frac{\left(- g \ell_{1} \left(m_{1} + m_{2}\right) \sin{\left(q_{1} \right)} - \ell_{1} \ell_{2} m_{2} v_{2}^{2} \sin{\left(q_{1} - q_{2} \right)}\right) \cos{\left(q_{1} - q_{2} \right)}}{\ell_{1} \ell_{2} m_{1} - \ell_{1} \ell_{2} m_{2} \cos^{2}{\left(q_{1} - q_{2} \right)} + \ell_{1} \ell_{2} m_{2}}\end{pmatrix} $$

*All formulas here are calculated using `sympy`. With helper variables it would probably be much easier to read.*
