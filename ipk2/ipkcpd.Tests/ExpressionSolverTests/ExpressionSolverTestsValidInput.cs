namespace ipkcpd.Tests.ExpressionSolverTests;

public class ExpressionSolverTestsValidInput
{
    [Theory]
    [InlineData("(+ 2 2)", 4)]
    [InlineData("(* 3 4)", 12)]
    [InlineData("(/ 10 2)", 5)]
    [InlineData("(- 8 5)", 3)]
    [InlineData("(* 2 3 4)", 24)]
    [InlineData("(- 7 3 1)", 3)]
    [InlineData("(+ 5 (* 3 2))", 11)]
    [InlineData("(/ 16 2 2)", 4)]
    [InlineData("(+ 1 (/ 5 5))", 2)]
    [InlineData("(+ (- 2 1) (* 1 1))", 2)]
    [InlineData("(- 10 (/ 20 2))", 0)]
    [InlineData("(* 2 3 4 5)", 120)]
    [InlineData("(/ 100 2 2 5)", 5)]
    [InlineData("(+ 1 (* 2 3) (/ 4 2))", 9)]
    [InlineData("(- 12 4 2 1)", 5)]
    [InlineData("(* 1 2 3 4 5)", 120)]
    [InlineData("(/ 1000 5 2 2 5)", 10)]
    [InlineData("(+ 2 (* 3 4) (- 10 6))", 18)]
    [InlineData("(- 100 50 20 10)", 20)]
    [InlineData("(* 2 (+ 3 4) (- 5 1))", 56)]
    [InlineData("(/ 200 (+ 100 50) (* 2 2))", 0)]
    [InlineData("(* (- 4 2) (+ 3 4) (/ 10 2))", 70)]
    [InlineData("(+ (* 2 3) (/ 8 2) (- 10 6))", 14)]
    [InlineData("(* (- 5 2) (+ 1 2 3) (/ 18 3))", 108)]
    [InlineData("(/ (- 10 6) (+ 1 2 3) (* 3 4))", 0)]
    [InlineData("(- (* 2 3 4) (/ 16 2) (+ 5 4))", 7)]
    [InlineData("(+ (/ 10 2) (* (- 3 4) (+ 1 2)))", 2)]
    [InlineData("(* (+ 3 4 5) (- 8 6) (/ 20 4))", 120)]
    [InlineData("(/ (+ (/ (+ (/ 121 (* (/ 2048 64 2) 5 3 2 18)) (* 2 (* 2 7 9))) (* (/ 2048 64 2) 5 3 2 18)) (* 64 7) (- 879 1321)) 2)", 3)]
    public void ValidInputs_ReturnExpectedResult(string input, int expected)
    {
        // Arrange
        ExpressionSolver solver = new();

        // Act
        var result = solver.Solve(input);

        // Assert
        Assert.Equal(expected, result);
    }

}