namespace ipkcpd.Tests.ExpressionSolverTests;

public class ExpressionSolverTestsInvalidInput
{
    [Theory]
    [InlineData("")]
    [InlineData("      ")]
    [InlineData("(+ 1)")]
    [InlineData("(   *    4   5   )     ")]
    [InlineData("(*    (* 4 5  (* 4 5) )     ")]
    [InlineData("(* -4 5)")]
    [InlineData("(* --4 5)")]
    [InlineData("(* - 4 5)")]
    [InlineData("(*(/ 4 4))HELLO")]
    [InlineData("(/ 5 0)")]
    [InlineData("(/ 5 (- 3 3))")]
    [InlineData("(/ 0 0)")]
    [InlineData("(/ 0 (- 3 3))")]
    [InlineData("(+ 1 2) ")]
    [InlineData(" (+ 1 2)")]
    [InlineData("(+ 1 2")]
    [InlineData("+ 1 2)")]
    [InlineData("(+ 1 )")]
    [InlineData("(1 2)")]
    public void InvalidInput_ThrowInvalidExpressionException(string expr)
    {
        // Act and Assert
        Assert.Throws<InvalidExpressionException>(() => new ExpressionSolver().Solve(expr));
    }
}
