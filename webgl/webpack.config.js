
module.exports = {
  mode: 'development',
  entry: './src/main.ts',
  output: {
    path: __dirname + '/public',
    filename: 'bundle.js'
  },
  module: {
    rules: [
      {
        test: /\.ts$/,
        loader: 'ts-loader'
      }
    ]
  },
  resolve: {
    extensions: ['.ts', '.js']
  },
  devServer: {
    contentBase: __dirname + '/public',
    port: 8010
  }
}