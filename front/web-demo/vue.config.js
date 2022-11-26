module.exports = {
  devServer: {
    proxy: {
      '/api': {
        target: 'http://snap-air-unit.local:80',
        changeOrigin: true,
        ws: true
      }
    }
  }
}
