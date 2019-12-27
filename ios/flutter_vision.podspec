#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html
#
Pod::Spec.new do |s|
  s.name             = 'flutter_vision'
  s.version          = '0.0.1'
  s.summary          = 'A flutter plugin use google vision api'
  s.description      = <<-DESC
A flutter plugin use google vision api
                       DESC
  s.homepage         = 'http://example.com'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'Your Company' => 'email@example.com' }
  s.source           = { :path => '.' }
  # s.exclude_files = 'Classes/GiangLib/Utilities/**'
  s.source_files = 'Classes/**/*'
  s.public_header_files = 'Classes/**/*.h'
  s.private_header_files = 'Classes/GiangLib/Utilities/**.h'
  s.static_framework = true
  s.dependency 'OpenCV', '~> 3.1.0.1'
  s.dependency 'Flutter'

  s.ios.deployment_target = '8.0'
end

